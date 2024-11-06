#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <vector>
#include <chrono>

// Classe TicTacToe
class TicTacToe {
private:
    std::vector<std::vector<char>> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Rstado do jogo
    char winner; // Vencedor do jogo

public:
   TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
        current_player = 'X';
        game_over = false;
        winner = ' ';
    }


    void display_board() {
        // Exibir o tabuleiro no console

        // Esperar 300ms para exibir o tabuleiro
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif

        std::cout << "Tabuleiro atual:\n";
        
        for (size_t i = 0; i < board.size(); ++i) {
            for (size_t j = 0; j < board[i].size(); ++j) {
                std::cout << (board[i][j] == ' ' ? '.' : board[i][j]);

                if (j < board[i].size() - 1) {
                    std::cout << " | ";
                }
            }

            std::cout << "\n-----------\n";
        }

        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos

        std::lock_guard<std::mutex> lock(board_mutex);

        if (row < 0 || row >= 3 || col < 0 || col >= 3 || board[row][col] != ' ') {
            
            return false;
        }

        board[row][col] = player;
        display_board();

        if (check_win(player)) {
            winner = player;
            game_over = true;
            turn_cv.notify_all();

            return true;
        } else if (check_draw()) {
            winner = 'D';
            game_over = true;
            turn_cv.notify_all();
        }

        alternar_jogador();
        
        return false;
    }

    bool check_win(char player) {
        // Verificar se o jogador atual venceu o jogo

        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                
                return true;
            }
        }

        return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
               (board[0][2] == player && board[1][1] == player && board[2][0] == player);
    }

    bool check_draw() {
        // Verificar se houve um empate

        for (const auto& row : board) {
            for (const auto& cell : row) {
                if (cell == ' ') {
                    return false;
                }
            }
        }
        
        return true;
    }

    bool is_game_over() {
        // Retornar se o jogo terminou

        std::lock_guard<std::mutex> lock(board_mutex);
        
        return game_over;
    }

    char get_winner() { 
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)

        return winner; 
    }

    void alternar_jogador() {
        // Alternar o jogador atual (criada)

        current_player = (current_player == 'X') ? 'O' : 'X';
        turn_cv.notify_all();
    }

    void esperar_turno(char player) {
        // Esperar o turno do jogador (criada)

        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [&] { return current_player == player || game_over; });
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        // Executar jogadas de acordo com a estratégia escolhida

        if (strategy == "sequencial") {
            play_sequential();
        } else if (strategy == "aleatorio") {
            play_random();
        }
    }

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas

        for (int i = 0; i < 3 && !game.is_game_over(); ++i) {
            for (int j = 0; j < 3 && !game.is_game_over(); ++j) {
                game.esperar_turno(symbol);
                
                if (game.is_game_over()) {
                    return;
                }

                if (game.make_move(symbol, i, j)) {
                    return;
                }
            }
        }
    }

    void play_random() {
        // Implementar a estratégia aleatória de jogadas

        std::srand(std::time(nullptr));
        while (!game.is_game_over()) {
            game.esperar_turno(symbol);
            if (game.is_game_over()) {
                return;
            }

            int row = std::rand() % 3;
            int col = std::rand() % 3;

            if (game.make_move(symbol, row, col)) {
                return;
            }
        }
    }
};

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe game;
    Player playerX(game, 'X', "sequencial");
    Player playerO(game, 'O', "aleatorio");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &playerX);
    std::thread t2(&Player::play, &playerO);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogoS
    if (game.is_game_over()) {
        char winner = game.get_winner();
        if (winner == 'D') {
            std::cout << "O jogo terminou EMPATADO.\n";
        } else {
            std::cout << "Jogador " << winner << " ganhou o jogo!\n";
        }
    }

    return 0;
}
