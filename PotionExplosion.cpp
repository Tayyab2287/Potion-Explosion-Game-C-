#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iomanip>
using namespace std;

const int DISPENSER_ROWS = 7;
const int DISPENSER_COLS = 5;
const int PLAYER_BOARDS = 2;
const int PLAYER_BOARD_COLS = 5;

// Shared Memory IDs
int shmid_dispenser;
int shmid_playerBoard[PLAYER_BOARDS];
int shmid_playerScores;

// using Mutex
pthread_mutex_t mutex_dispenser = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_playerBoard[PLAYER_BOARDS] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

enum GameState
{
    START,
    PLAYING,
    OVER
};
GameState gameState = START;


const char PLAYER_COLORS[PLAYER_BOARDS] = {'R', 'B'};
const char RED = 'R';
const char YELLOW = 'Y';
const char BLUE = 'B';
const char BLACK = 'K';
char *dispenser;
char *playerBoard[PLAYER_BOARDS];
int *playerScores;

void checkReaction(int player);

template <typename T>
T *createSharedMemory(int &shmid, int size)
{
    shmid = shmget(IPC_PRIVATE, sizeof(T) * size, IPC_CREAT | 0666);
    T *memory = (T *)shmat(shmid, NULL, 0);
    return memory;
}

void initializeSharedMemory()
{
    // random number generator
    srand(time(NULL));
    dispenser = createSharedMemory<char>(shmid_dispenser, DISPENSER_ROWS * DISPENSER_COLS);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        playerBoard[i] = createSharedMemory<char>(shmid_playerBoard[i], PLAYER_BOARD_COLS);
    }
    playerScores = createSharedMemory<int>(shmid_playerScores, PLAYER_BOARDS);
    for (int i = 0; i < DISPENSER_ROWS * DISPENSER_COLS; ++i)
    {
        dispenser[i] = rand() % 4 + 'A'; // Using 'A', 'B', 'C', 'D' randomly as ingredients
    }
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        for (int j = 0; j < PLAYER_BOARD_COLS; ++j)
        {
            playerBoard[i][j] = ' ';
        }
    }
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        playerScores[i] = 0;
    }
}

//code for clearing the terminal
void clearTerminal()
{
    std::cout << "\033[2J\033[1;1H";
}




void displayDispenser()
{
    clearTerminal();
    pthread_mutex_lock(&mutex_dispenser);
    cout<< "\033[1;34m"<<"               PPPP   OOO   TTTTT III  OOO  N   N      EEEEE X   X PPPP  L     OOO   SSSS  III  OOO  N   N"<<endl;
    cout<<"               P   P O   O    T    I  O   O NN  N      E      X X  P   P L    O   O  s      I  O   O NN  N"<<endl;
    cout<<"               PPPP  O   O    T    I  O   O N N N      EEEEE   x   PPPP  L    O   O  SSSs   I  O   O N N N"<<endl;
    cout<<"               P     O   O    T    I  O   O N  NN      E      X X  P     L    O   O     S   I  O   O N  NN"<<endl;
    cout<<"               P      OOO     T   III  OOO  N   N      EEEEE X   X P     LLLL  OOO   SSSS  III  OOO  N   N"<< "\033[0m"<<endl;

   
    for(int i=0;i<5;i++)
    {
      cout<<endl;
    }
    cout <<setw(20)<< "\033[1;31m"<< "ooooooooooooooooooooo"<< "\033[0m" << endl;
    cout <<setw(20)<< "\033[1;31m"<< "o     DISPENSER     o"<< "\033[0m" <<"               "<<" RULES TO PLAY GAME"<< endl;
    cout <<setw(20)<< "\033[1;31m"<< "ooooooooooooooooooooo"<< "\033[0m" << endl;
    cout<<"             "<<"---------------------"<<endl;
    for (int i = 0; i < DISPENSER_ROWS; i++)
    {
        cout <<setw(15)<<" | ";
        for (int j = 0; j < DISPENSER_COLS; j++)
        {
            char ingredient = dispenser[i * DISPENSER_COLS + j];
            // Add color codes for different ingredients
            switch (ingredient)
            {
            case 'A':
                cout << "\033[1;31m" << ingredient << "\033[0m"<<" | "; // Red
                break;
            case 'B':
                cout << "\033[1;33m" << ingredient << "\033[0m"<<" | "; // Yellow
                break;
            case 'C':
                cout << "\033[1;32m" << ingredient << "\033[0m"<<" | "; // Green
                break;
            case 'D':
                cout << "\033[1;34m" << ingredient << "\033[0m"<<" | "; // Blue
                break;
            default:
                cout << ingredient << " | ";
            }
        }
       
         if(i==0)
        {
           cout<<"               "<<"1:Select the ingredient from the dispenser by specifying row number. ";
        }
         if(i==1)
        {
           cout<<"               "<<"2:Ingredients will be selected from the top of the dispenser";
        }
         if(i==2)
        {
           cout<<"               "<<"3:Selected ingredients will be removed from the dispenser.";
        }
         if(i==3)
        {
           cout<<"               "<<"4:You have to collect the ingredient to fill the flask.";
        }
         if(i==4)
        {
           cout<<"               "<<"  Every flask must contain the specific ingredient of quantity 3";
        }
         if(i==5)
        {
           cout<<"               "<<"6:Score will be increased by certain number each time player fills the flask.";
        }
        if(i==6)
        {
           cout<<"               "<<"7:at the end of the game player with more score wins";
        }
        cout << endl;
    }
    cout<<"             "<<"---------------------"<<endl;
    for(int i=0;i<3;i++)
    {
      cout<<endl;
    }
    pthread_mutex_unlock(&mutex_dispenser);
}

void displayPlayerInfo()
{
    for (int player = 0; player < PLAYER_BOARDS; player++)
    {
        pthread_mutex_lock(&mutex_playerBoard[player]);
        if (player % 2 == 0)
        {
            cout << "\033[1;35m"; // pink color for Player 1
        }
        else
        {
            cout << "\033[1;33m"; // Yellow color for Player 2
        }
        cout << "Player " << (player + 1) << "'s Board: "<< "\033[0m";
        for (int j = 0; j < PLAYER_BOARD_COLS; j++)
  {
     cout << playerBoard[player][j] << " ";
  }
 
if(player>0)
{
           cout<<endl;
           cout << "\033[1;35m"<< "Score: " << playerScores[player-1] << "\033[0m"<<"                    ";
           cout << "\033[1;33m"<< "Score: " << playerScores[player] << "\033[0m"<<endl;
        }
        pthread_mutex_unlock(&mutex_playerBoard[player]);
    }
}

int getPlayerMove(int player)
{
    int move;
    while (true)
    {
    cout<<endl;
    if(player==0)
    {
     cout << "Player " << "\033[1;35m"<< (player + 1)<< "\033[0m" << " turn enter your move (1-5): ";
    }
    else
    {
          cout << "Player " << "\033[1;33m"<< (player + 1)<< "\033[0m" << " turn enter your move (1-5): ";
        }
        cin >> move;
        if (move >= 1 && move <= PLAYER_BOARD_COLS)
        {
            return move;
        }
        else
        {
            cout << "Invalid move. Please enter a number between 1 and 5." << endl;
        }
    }
}

void processMove(int move, int player)
{
    int col = move - 1;

    // Check if the move is valid
    if (dispenser[col] != ' ')
    {
        char ingredient = dispenser[col];
        for (int i = 0; i < DISPENSER_ROWS - 1; i++)
        {
            dispenser[i * DISPENSER_COLS + col] = dispenser[(i + 1) * DISPENSER_COLS + col];
        }
        dispenser[(DISPENSER_ROWS - 1) * DISPENSER_COLS + col] = ' ';
        for (int i = 0; i < PLAYER_BOARD_COLS; i++)
        {
            if (playerBoard[player][i] == ' ')
            {
                playerBoard[player][i] = ingredient;
                break;
            }
        }
        checkReaction(player);
    }
    else
    {
        cout << "Invalid move. Please choose a column with marbles." << endl;
    }
}

void checkReaction(int player)
{
    int count = 0;
    char lastIngredient = ' ';
    for (int i = 0; i < PLAYER_BOARD_COLS; i++)
    {
        if (playerBoard[player][i] == ' ')
        {
            break;
        }
        if (playerBoard[player][i] == lastIngredient)
        {
            count++;
        }
        else
        {
            count = 1;
            lastIngredient = playerBoard[player][i];
        }      
        if (count >= 3)
        {
            // Removing ingredients from the player's board
            for (int j = i; j >= i - 2; j--)
            {
                playerBoard[player][j] = ' ';
            }
            if (lastIngredient == RED)
            {
                playerScores[player] += 2;
            }
            else if (lastIngredient == YELLOW)
            {
                playerScores[player] += 3;
            }
            else if (lastIngredient == BLUE)
            {
                playerScores[player] += 4;
            }
            else
            {
                playerScores[player] += 5;
            }
            checkReaction(player);
        }
    }
}

bool isGameOver()
{
    for (int j = 0; j < DISPENSER_COLS; j++)
    {
        if (dispenser[j] != ' ')
        {
            return false;
        }
    }
    return true;
}

void declareWinner()
{
    displayPlayerInfo();
    if (playerScores[0] > playerScores[1])
    {
        cout << "Player 1 wins!" << endl;
    }
    else if (playerScores[1] > playerScores[0])
    {
        cout << "Player 2 wins!" << endl;
    }
    else
    {
        cout << "It's a tie!" << endl;
    }
}

// Function for the game loop in a separate thread
void *gameLoop(void *arg)
{
    while (!isGameOver())
    {
        displayDispenser();
        displayPlayerInfo();
        int move1 = getPlayerMove(0);
        processMove(move1, 0);
        displayDispenser();
        displayPlayerInfo();
        if (isGameOver())
        {
            break;
        }
        int move2 = getPlayerMove(1);
        processMove(move2, 1);
    }
    declareWinner();
    shmdt(dispenser);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        shmdt(playerBoard[i]);
    }
    shmdt(playerScores);
    shmctl(shmid_dispenser, IPC_RMID, NULL);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        shmctl(shmid_playerBoard[i], IPC_RMID, NULL);
    }
    shmctl(shmid_playerScores, IPC_RMID, NULL);
    pthread_exit(NULL);
}

int main()
{
    initializeSharedMemory();
    pthread_t thread;
    pthread_create(&thread, NULL, gameLoop, NULL);
    pthread_join(thread, NULL);
    return 0;
}

