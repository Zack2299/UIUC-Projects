import chess
import io

# run command "pip install stockfish"
from stockfish import Stockfish

skip_camera = None
ai_depth = 1
def init_stockfish(bool1, difficulty):
    global skip_camera
    global ai_depth
    if difficulty == 'easy':
        ai_depth = 5
    elif difficulty == 'medium':
        ai_depth = 10
    elif difficulty == 'hard':
        ai_depth = 15
    skip_camera = bool1

# def read_from(file_name):
#     with open(r"%s" %file_name, 'r') as File_object: # 'with' autocloses file
#         file_data = File_object.read()
#     return file_data

# def write_to(file_name, file_data):
#     with open(r"%s" %file_name, 'w') as File_object:
#         File_object.write(file_data)
#     return 0

def FEN_check(FEN_string):
    '''
    DESCRIPTION:    Checks if the FEN_string is valid.
    INPUT:          FEN_string: String in FEN format.
    RETURN:         is_valid_FEN: Bool that says if the FEN_string was valid.
    '''
    try:
        board = chess.Board(FEN_string)
        return True
    except ValueError:
        return False

def cheat_check(FEN_string, FEN_board_state_before):
    # flag variable
    is_cheating = True

    # # set board and FEN from state before
    # FEN_board_state_before = read_from("saved_FEN")
    board = chess.Board(FEN_string)
    curr_ply = board.ply()
    # print("This is the ply:", curr_ply)

    # Edge case: AI is first to move on start
    if (FEN_board_state_before == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" or FEN_board_state_before == "" and curr_ply < 2):
        is_cheating = False
        return is_cheating


    if FEN_string == "":
        return is_cheating
    
    board_before = chess.Board(FEN_board_state_before)

    # start of weird/custom game, ignore on the first time
    if board_before.ply() < 2:
        is_cheating = False
        return is_cheating

    # possible move set from board state before
    move_list = board_before.legal_moves

    # iterate all moves to find a move that makes curr board state
    for move in move_list:
        # make copy of board state before and push moves
        board_copy = board_before.copy()
        board_copy.push(move)

        # if there's a match, it's legal
        if (board_copy.fen() == FEN_string):
            is_cheating = False
            break

    return is_cheating

def special_move_check(FEN_string, best_move, is_cheating):
    '''
    DESCRIPTION:    Check if the move is capture, castling, promotion, or en passant.
    INPUT:          FEN_string: String that holds FEN.
                    best_move: String that holds move
    RETURN:         special_move_return: List holding flags of special moves and location of affected piece:
                        [0~5]: Hold 0/1 (False/ True) signifying what type of move.
                            0: Not a special move
                            1: En Passant
                            2: Castling
                            3: Capture
                            4: Promotion
                            5: Cheating
                        [6]: Holds best move.

    '''
    special_move_return = [0, 0, 0, 0, 0, 0, best_move]
    # print("This is best move:", best_move)
    board = chess.Board(FEN_string)
    move_processed = chess.Move.from_uci(best_move)
    
    # check en passant
    if board.is_en_passant(move_processed):
        special_move_return[1] = 1

    # check castling
    if board.is_castling(move_processed):
        special_move_return[2] = 1

    # check capture
    if board.is_capture(move_processed):
        special_move_return[3] = 1

    # check promotion
    is_promotion = chess.square_rank(chess.Move.from_uci(best_move).to_square) in [0, 7] and board.piece_type_at(chess.Move.from_uci(best_move).from_square) == chess.PAWN
    if is_promotion:
        special_move_return[4] = 1

    # check cheating
    if is_cheating:
        special_move_return[5] = 1

    return special_move_return

def Stockfish_eval(FEN_string):
    '''
    DESCRIPTION:    Calculates a best move based off Stockfish engine.
    INPUT:          FEN_string: String in FEN notation.          
    RETURN:         best_move: String of best move (i.e. "e7e6").
    '''

    # setting up stockfish
    if skip_camera:
        stockfish = Stockfish(  path = "C:\ece445\stockfish\stockfish-windows-x86-64-avx2.exe", # install Stockfish and input path to .exe here
                                depth = ai_depth,                          # 20 is good but slower, 15 is fine
                                parameters={"Skill Level":19})    # range: 0-19? 20 might exist or rounds to 19, https://github.com/vondele/Stockfish/blob/master/src/search.cpp
    else:
        # print("RASPI STOCKFISH")
        stockfish = Stockfish(  path = "/home/Team33/Documents/Stockfish/arm_stockfish/stockfish/stockfish-android-armv8", # install Stockfish and input path to .exe here
                                depth = ai_depth,                          # 20 is good but slower, 15 is fine
                                parameters={"Skill Level":19})
        
    # set state of board based off FEN
    stockfish.set_fen_position(FEN_string)

    # eval best move
    best_move = stockfish.get_best_move()

    # === TEST: Visualize chess board state ===
    # print(stockfish.get_board_visual(True))

    return best_move

def Minimax_eval(FEN_string):
    return 0

def Personal_eval(FEN_string):
    return 0

def eval_best_move(FEN_string, which_algorithm):
    '''
    DESCRIPTION:    Takes in current game state as FEN and then evaluates a best move.
    INPUT:          FEN_string: A FEN string.
                    which_algorithm: Flag that determines best move algorithm:
                        0: Stockfish
                        1: Minimax
                        2: Personal
    RETURN:         best_move: Tuple that holds:
                        [0]: From move info.
                        [1]: To move info.
                        [2]: FEN string of board.
    '''    
    # calc best move based off flag
    best_move = [Stockfish_eval(FEN_string), Minimax_eval(FEN_string), Personal_eval(FEN_string)][which_algorithm]

    # === TEST: See best move ===
    # print("Best move is: ", best_move)

    return best_move

def prep_info_to_ESP(FEN_string, best_move_processed):
    '''
    DESCRIPTION:    Preps the info to be sent to the ESP32.
    INPUTS:         FEN_string: String that holds FEN.
                    best_move_processed: List that holds flags and best move.
    RETURN:         ESP_input: List containing info for the ESP32.
                        [0]: Contains move to go to.
                        [1]: Contains various strings of secondary piece, if applicable.
                            "": If reg move or promotion.
                            "<move>": If En Passant, Castling, or Capture.
                        [2]: Contains special move flags.
                            [0]: Reg move
                            [1]: En Passant
                            [2]: Castling
                            [3]: Capture
                            [4]: Promotion
                        [3]: Contains FEN before move.
    
    '''
    # print(best_move_processed)
    special_piece_location = ""

    # Promotion needs no case since:
        # if there's no piece in its way, it's a simple move
        # if there's a piece, it will go under Capture

    # Capture comes before En Passant since En Passant captures a piece in the process
    # Capture case
    if best_move_processed[3]:
        special_piece_location = [best_move_processed[6][2] + ' ' + best_move_processed[6][3]]

    # En Passant case
    if best_move_processed[1]:
        # left to right
        special_piece_location = [best_move_processed[6][2] + ' ' + best_move_processed[6][1]]
    
    # Castling case
    if best_move_processed[2]:
        # king side castle (e -> g)
        # if best_move_processed[5][2] == 'g':
        special_piece_location = [('h' if best_move_processed[6][2] == 'g' else 'a') + ' ' + best_move_processed[6][1], ('f' if best_move_processed[6][2] == 'g' else 'd') + ' ' + best_move_processed[6][1]] 

        # queen side castle (e -> c)
        # if best_move_processed[5][2] == 'c':
        
    ESP_input = [[best_move_processed[6][0] + ' ' + best_move_processed[6][1], best_move_processed[6][2] + ' ' + best_move_processed[6][3]], 
                 special_piece_location, 
                 best_move_processed[0:6], 
                 FEN_string]

    # print(ESP_input)

    # get move's dest and src (separated with space)
    # move_from = best_move[0:1] + ' ' + best_move[1:2]
    # move_to = best_move[2:3] + ' ' + best_move[3:4]


    return ESP_input

# def update_FEN_string(FEN_string, best_move, file_name):
#     '''
#     DESCRIPTION:    Updates the saved FEN with the best_move added to it.
#     INPUT:          FEN_string: String that hold FEN notation of board.
#                     best_move: String that holds best move from eval.
#                     file_name: String that holds the file name.
#     RETURN:         n/a
#     '''
#     board = chess.Board(FEN_string)   
#     board.push(chess.Move.from_uci(best_move))
#     write_to(file_name, board.fen())

def chess_AI(FEN_string, FEN_board_state_before, which_alg):
    # check if valid FEN
    is_valid_FEN = FEN_check(FEN_string)

    if not is_valid_FEN:
        print("FEN format is wrong, please fix")
        return -1
   
    else:
        # # check if there's cheating
        # is_cheating = cheat_check(FEN_string, FEN_board_state_before)
        
        # if is_cheating:
        #     print("Illegal move detected, please fix")
        #     return -1
        # else:
        
        # get best move
        best_move = eval_best_move(FEN_string, which_alg)

        # check move for special characteristics
        best_move_processed = special_move_check(FEN_string, best_move, False)

        # prepare info for ESP32
        ESP_input = prep_info_to_ESP(FEN_string, best_move_processed)

        # # update saved FEN 
        # file_name = "saved_FEN"
        # update_FEN_string(FEN_string, best_move, file_name)

        return ESP_input, best_move

# def tests(FEN_string, which_func, which_test):
#     '''
#     DESCRIPTION:    Contains and executes different tests for various functions.    
#     INPUT:          FEN_string: String containing FEN.
#                     which_func: Int that dictates which functionality to test for.
#                         0: FEN
#                         1: Eval
#                         2: Cheating
#                     which_test: Int that dictates which test of specific functionality to test for.
#     RETURN:
#     '''
#     message = "FEN Parsing" if which_func == 0 else "Eval" if which_func == 1 else "Cheating" if  which_func == 2 else "Special Move" if which_func == 3 else "Processing ESP32 Info" if which_func == 4 else "Chess AI"
#     print("You're currently testing: " + message + "'s Test #" + str(which_test))

#     # === TESTS: FEN Parsing ===
#     if which_func == 0:
#         is_valid_FEN = False

#         # 0: Valid FEN (Success)
#         if which_test == 0:
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#             is_valid_FEN = FEN_check(FEN_string)
        
#         # 1: Invalid FEN (Fail)
#         if which_test == 1:
#             FEN_string = "This should totally fail"
#             is_valid_FEN = FEN_check(FEN_string)

#         # 2: Invalid FEN by 1 extra char (Fail)
#         if which_test == 2:
#             FEN_string = "rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#             is_valid_FEN = FEN_check(FEN_string)

#         message = "Valid" if is_valid_FEN else "Invalid"
#         print("This inputted FEN is:", message)
        
#     # === TESTS: Eval ===
#     if which_func == 1:
        
#         # TEST: Inputting different moves ===
#         if which_test == 0:
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts
#             board = chess.Board(FEN_string)
#             move_list = ["g1f3", "c7c5"          # "b8a6"
#                         ]
#             for move in move_list:
#                 board.push(chess.Move.from_uci(move))

#     # === TESTS: Cheating ===
#     if which_func == 2:
#         # save old FEN
#         file_name = "saved_FEN"
#         saved_FEN = read_from(file_name)
#         is_cheating = True

#         # 0: Illegal ply/state of board (Fail)
#         if which_test == 0:
#             write_to(file_name, "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 1 6")  # random board state
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts
#             is_cheating = cheat_check(FEN_string)

#         # 1: Valid ply by human (Success)
#         if which_test == 1:
#             write_to(file_name, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") # base: white starts
#             FEN_string = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1" # black's turn, one ply by white since base
#             is_cheating = cheat_check(FEN_string)

#         # 2: AI is first to move on start & saved_FEN is filled (Success)
#         if which_test == 2:
#             write_to(file_name, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") # base: white starts
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts
#             is_cheating = cheat_check(FEN_string)

#         # 3: AI is first to move on start & saved_FEN is empty (Success)
#         if which_test == 3:
#             write_to(file_name, "") # base: white starts
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts
#             is_cheating = cheat_check(FEN_string)

#         message = "Yes" if is_cheating else "No"
#         print("Was there cheating involved?:", message)

#         # restore old FEN
#         write_to(file_name, saved_FEN)

#     # === TESTS: Special Moves ===
#     if which_func == 3:
        
#         # 0: Castle
#         if which_test == 0:
#             is_special_move = special_move_check("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", "e1g1")

#         # 1: Capture
#         if which_test == 1:
#             is_special_move = special_move_check("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3", "f3e5")

#         # 2: Promotion
#         if which_test == 2:
#             is_special_move = special_move_check("8/PPPPP1PP/8/8/8/8/8/8 w - - 0 1", "a7a8q")
        
#         # 3: En Passant & Capture
#         if which_test == 3:
#             is_special_move = special_move_check("8/8/8/4pP2/4P3/8/8/8 w - e6 0 1", "f5e6")
        
#         # print messages based off what it was
#         if is_special_move[0]:
#                 print("The move results in reg move")
#         if is_special_move[1]:
#                 print("The move results in En Passant at location:", is_special_move[5][2:4])
#         if is_special_move[2]:
#                 print("The move results in Castling at location:", is_special_move[5][2:4])
#         if is_special_move[3]:
#                 print("The move results in Capture at location:", is_special_move[5][2:4])
#         if is_special_move[4]:
#                 print("The move results in Promotion at location:", is_special_move[5][2:4])

#     # === TESTS: Process ESP32 ===
#     if which_func == 4:

#         # 0: En Passant
#         if which_test == 0:
#             FEN_string = "8/8/8/4pP2/4P3/8/8/8 w - e6 0 1"
#             best_move_processed = special_move_check("8/8/8/4pP2/4P3/8/8/8 w - e6 0 1", "f5e6")
#             ESP_input = prep_info_to_ESP(FEN_string, best_move_processed)

#         # 1: Capture
#         if which_test == 1:
#             FEN_string = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3"
#             best_move_processed = is_special_move = special_move_check("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3", "f3e5")
#             ESP_input = prep_info_to_ESP(FEN_string, best_move_processed)

#         # 2: Castling (king side)
#         if which_test == 2:
#             FEN_string = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"
#             best_move_processed = special_move_check("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", "e1g1")
#             ESP_input = prep_info_to_ESP(FEN_string, best_move_processed)
        
#         # 3: Castling (queen side)
#         if which_test == 3:
#             FEN_string = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#             best_move_processed = special_move_check("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1c1")
#             ESP_input = prep_info_to_ESP(FEN_string, best_move_processed)

#         print(ESP_input)

#     # === TESTS: Entire Alg ===
#     if which_func == 5:
#         file_name = "saved_FEN"
#         saved_FEN = read_from(file_name)

#         # 0: Base game
#         if which_test == 0:
#             FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts

#         if which_test == 1:
#             FEN_string = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"  # grabbed from ESP32, Test 3
        
#         board = chess.Board(FEN_string)
#         print(board)

#         which_alg = 0
#         ret_val = chess_AI(FEN_string, which_alg)
#         print("Return value is:",ret_val)
#         string_test = ret_val[0][0] + ret_val[0][1]
#         string_test = string_test.replace(" ", "")
#         board.push(chess.Move.from_uci(string_test))
#         print(board)

#         # restore old FEN
#         write_to(file_name, saved_FEN)

# if __name__ == '__main__':  
#     # comment out these when calling with Raspberry PI 4 (make sure when you call chess_AI, you set the parameters)
#     FEN_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # base: white starts
#     # FEN_string = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
#     which_alg = 0

#     # Chess AI program
#     # ret_val = chess_AI(FEN_string, which_alg)
#     # print(ret_val)

#     # Tests
#     which_func = 5
#     which_test = 1
#     tests(FEN_string, which_func, which_test)