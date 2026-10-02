import argparse
import chess
import chess.pgn
from board_detector import find_board
from board_detector import warp_board
from board_detector import find_pieces
from board_detector import init_global
from board_detector import display_img
from chess_ai import *
import move_translator
import cv2
import os
import time
import easyocr
import sys
from planning import Path_planning

# was having issues installing picamera2 on my PC
# if the raspi runs this code (with picamera2 installed), skip_camera will be false
# otherwise skip_camera is true and any camera code will be skipped
skip_camera = False
try:
    from picamera2 import Picamera2, Preview
except ImportError:
    skip_camera = True

example_fens = {"gunnar_enpass": "r1bq1r2/pp2n1p1/4N2k/3pPp1P/1b1n2Q1/2N5/PP3PP1/R1B1K2R b KQ - 0 14",
                "piece_test_castling1-": "r3k3/1b1p1p2/p3pK2/5qp1/2P5/P3P3/1P2B3/R1R1Q3 w q - 4 30"} # prins 

class ChessGame:
    def __init__(self, difficulty, color_scheme, show_cv, show_board, enable_castling, mag_arm_enable, show_cam, loop, img_idx = 1, test_img = None, save_img_as = None):
        self.board = chess.Board()
        self.prev_board = chess.Board()
        self.difficulty = difficulty
        self.color_scheme = color_scheme
        self.show_cv = show_cv
        self.show_board = show_board
        self.enable_castling = enable_castling
        self.mag_arm_enable = mag_arm_enable
        self.show_cam = show_cam
        self.test_img = test_img
        self.loop = loop
        self.save_img_as = save_img_as
        self.mychess = Path_planning(23)
        if img_idx:
            self.img_idx = int(img_idx)
        else:
            self.img_idx = 1

        self.player_cheating = False

        self.reader = easyocr.Reader(['en'], gpu=False, verbose=False)

        self.best_move = None

        # hard-coded borders to crop image
        self.left_cut = 0
        self.right_cut = 280
        self.top_cut = 0
        self.bottom_cut = 280

        self.img_size = 512

        # store warping info
        self.corners_sorted = None
        self.intersection = None
        self.drawn_warped_img = None
        self.prev_warped_img = None
        self.warped_img = None

        if not skip_camera and self.test_img is None:
            self.picam2 = Picamera2()
        else:
            self.picam2 = None
        # self.picam2 = Picamera2()

    def start_game(self):

        print(f"Starting chess game (difficulty: {self.difficulty})")

        # TODO - call initialize board in board_detector, initialize colors for color analysis,
        # then loop until checkmate. also handle illegal moves (writing to screen if we end up doing that or just LEDs)

        init_global(self.show_cv, self.show_board, skip_camera, self.img_size, self.enable_castling)
        init_stockfish(skip_camera, self.difficulty)

        if self.save_img_as:
            self.loop = True

        # init camera
        if not skip_camera and self.test_img is None:
            preview_config = self.picam2.create_preview_configuration(main={"size": (2464, 2464)})
            self.picam2.configure(preview_config)
            if (self.show_cam):
                self.picam2.start_preview(Preview.QTGL)
            self.picam2.start()
        elif not self.test_img:
            self.test_img = 'ocr_ps_pink_yellow.jpg'

        # initial setup of board
        print("\n--- STARTING BOARD ---")

        self.do_cv()
        self.prev_warped_img = self.warped_img
        # use example fen if not starting from blank game
        if self.test_img in example_fens:
            self.board = chess.Board(example_fens[self.test_img])
            print(self.board)
            print(self.board.fen())
        elif self.img_idx == 2:
            self.board = chess.Board()
            # self.img_idx += 1

        if self.show_board and self.drawn_warped_img is not None:
            display_img([self.drawn_warped_img])

        while(1): # game loop

            # if self.board.turn == chess.WHITE:
            self.player_turn()

            if self.show_board and self.drawn_warped_img is not None:
                display_img([self.drawn_warped_img])

            if self.check_game_over():
                break

            # if self.board.turn == chess.BLACK:
            self.ai_turn()

            if self.show_board and self.drawn_warped_img is not None:
                display_img([self.drawn_warped_img])

            if self.check_game_over():
                break

        # game is over

    # def switch_turn(self):
    #     if self.board.turn == chess.WHITE:
    #         print("Next move: Black")
    #         self.board.turn = chess.BLACK
    #     else:
    #         print("Next move: White")
    #         self.board.turn = chess.WHITE

    def player_turn(self):
        print("\n--- PLAYER'S TURN ---")
        # TODO - wait for user button, then check for valid move. loop until a valid move has been made
        input("[Waiting to submit player move. Replace with physical button]")
        # self.switch_turn()
        self.do_cv()
        # handle cheating
        while self.player_cheating:
            # print("CHECKING: ", self.board.fen())
            self.board = self.prev_board.copy()
            print("Player is cheating. Return to the previous state and perform a valid move.")
            self.img_idx -= 1
            # print(self.board)

            # ESP_input, _ = chess_AI(self.board.fen(), self.prev_board.fen(), self.player_cheating, 0)
            # ESP_input = [["_ _","_ _"],[]]

            # ----------------------------ADD JOSE LED STUFF
            if (self.mag_arm_enable and not skip_camera):
                self.mychess.sendCheating(self.player_cheating)

            input("[Waiting to submit player move. Replace with physical button]")
            self.do_cv()

        if not self.player_cheating:
            self.prev_warped_img = self.warped_img

    def ai_turn(self):
        print("\n--- CHESS ROBOT'S TURN ---")
        # print("Board before chess_AI:", self.board.fen())  
        ESP_input, self.best_move = chess_AI(self.board.fen(), self.prev_board.fen(), 0)

        print("Best Move: ", self.best_move)

        # convert best move to coordinates to send
         # ----------------------------ADD JOSE MOVE MAGNET STUFF
        if (self.mag_arm_enable and not skip_camera):
            self.mychess.chessExecuteMove(ESP_input)
            print("Magnetic arm is done moving.")
        else:
            input("[Waiting to submit chess robot's move (magnetic arm is disabled)]")
        # self.switch_turn()
        self.do_cv()
        self.prev_warped_img = self.warped_img

    def do_cv(self):
        while(1): # essentially do_while(self.loop)
            if (self.test_img):
                img_txt = self.test_img + str(self.img_idx) + '.jpg'
                img_path = os.path.join('ocr_test_images', img_txt)
                if not os.path.exists(img_path):
                    print("\n-------------------")
                    print("Out of test images.")
                    print("-------------------\n")
                    sys.exit()
                orig_img = cv2.imread(img_path)
                self.img_idx += 1
            else:
                orig_img = self.take_pic()
            h,w,c = orig_img.shape
            cropped_img = orig_img[self.top_cut:h-self.bottom_cut, self.left_cut:w-self.right_cut]
            # cropped_img = orig_img
            img = cv2.resize(cropped_img, (self.img_size, self.img_size))
            img = cv2.flip(img, -1) # flip vertically so black is on top
            # img = orig_img

            if (self.show_cv and not self.loop and False):
                # display_img([orig_img, img])
                display_img([img])

            if (self.loop):
                answer = 0
                while(answer != "y" and answer != "n"):
                    answer = input("Done looping? (y/n): ")
                    if answer == "y":
                        self.loop = False
                        sys.exit()
                    elif answer == "n":
                        self.loop = True

            if (not self.loop):
                break

        if self.img_idx == 2:
            self.corners_sorted, self.intersection = find_board(img)
        
        # warp the image based on the lines of the board
        # if self.img_idx == 1:
        self.warped_img, sorted_warped_points = warp_board(img, self.corners_sorted, self.intersection)

        if (self.warped_img is None):
            print("warped_img is None")
            return
        
        # get the pieces based on color thresholding and easyocr
        color_grid, self.drawn_warped_img = find_pieces(self.warped_img, self.prev_warped_img, sorted_warped_points, self.reader, self.img_idx - 1)

        # convert color_grid to board, which we can get fen string from
        if self.color_scheme == 'o/b':
            self.color_grid_to_fen(color_grid, 'Blue', 'Orange')
        elif self.color_scheme == 'p/y':
            self.color_grid_to_fen(color_grid, 'Yellow', 'Pink')
        else:
            self.color_grid_to_fen(color_grid, 'Red', 'Teal')

        if not self.enable_castling:
            self.board.set_castling_fen("-") # REMOVE CASTLING

        # check for invalid fen string
        if not FEN_check(self.board.fen()):
            print("Bad FEN.")
            return
        
        # print the board to terminal:
        if self.test_img not in example_fens or self.img_idx != 2:
            print(self.board)
            print(self.board.fen())

    def check_game_over(self):
        if self.board.is_checkmate():
            print("\n----------------------")
            if self.board.turn == chess.WHITE:
                print("Checkmate. Black wins!")
            else:
                print("Checkmate. White wins!")
            print("----------------------\n")
            return True
        elif self.board.is_stalemate():
            print("\n----------")
            print("Stalemate.")
            print("----------\n")
            return True
        elif self.board.is_insufficient_material():
            print("\n------------------------------")
            print("Draw by insufficient material.")
            print("------------------------------\n")
            return True
        return False

    def take_pic(self):
        time.sleep(2) # camera needs this apparently

        # name the image
        if (self.save_img_as):
            img_txt = self.save_img_as + str(self.img_idx) + '.jpg'
        else:
            img_txt = 'board' + str(self.img_idx) + '.jpg'

        # save image
        img_path = os.path.join('game_images', img_txt)
        metadata = self.picam2.capture_file(img_path)
        self.img_idx += 1
        return cv2.imread(img_path)

    def color_grid_to_fen(self, color_grid, color1, color2):
        print(color1, "(top) is black.", color2, "(bottom) is white.")
        
        temp_board = self.board.copy()
        self.prev_board = self.board.copy()
        for i, row in enumerate(color_grid):
            for j, (color, _, _, letter) in enumerate(row):
                piece_type = None
                if letter is not None:
                    # print("letter:", letter)
                    letter = letter[0]
                    if letter == "P":
                        piece_type = chess.PAWN
                    elif letter == "N":
                        piece_type = chess.KNIGHT
                    elif letter == "B":
                        piece_type = chess.BISHOP
                    elif letter == "R":
                        piece_type = chess.ROOK
                    elif letter == "Q":
                        piece_type = chess.QUEEN
                    elif letter == "K":
                        piece_type = chess.KING
                    elif letter == "X": # 'X - remove letter'
                        temp_board.remove_piece_at(chess.square(j, 7 - i))
                        continue

                    piece_color = None
                    if color == color1:
                        piece_color = chess.BLACK
                    elif color == color2:
                        piece_color = chess.WHITE
                    if piece_color is not None and piece_type is not None:
                        temp_board.set_piece_at(chess.square(j, 7 - i), chess.Piece(piece_type, piece_color))
                        continue

                if self.img_idx == 1:
                    temp_board.remove_piece_at(chess.square(j, 7 - i))

        # if temp_board.turn == chess.BLACK: # if the player just went
        #     cheat_check(temp_boself.prev_board.fen())
        
        # find move with correct fen string (with castling, en passant, half move counter, etc.)
        # Would be more efficient to update the extra fen string parameters myself. Might want to change
        move_list = self.prev_board.legal_moves
        found_move = False
        for move in move_list:
            board_copy = self.prev_board.copy()
            board_copy.push(move)

            if (temp_board.board_fen() == board_copy.board_fen()):
                found_move = True
                print("Found move!", move)
                self.board = board_copy.copy()
                if self.player_cheating:
                    self.player_cheating = False
                    if (self.mag_arm_enable and not skip_camera):
                        self.mychess.sendCheating(self.player_cheating)
                return
        
        if not found_move:
            if self.prev_board.turn == chess.WHITE: # if player just went, they cheated
                self.player_cheating = True
            else: # chess robot just went
                print("!")
                self.board.push(chess.Move.from_uci(self.best_move))
                # print("Did NOT find chess robot's move.")
                # if self.img_idx != 2:
                #     print("Detected Board: ")
                #     print(temp_board)
                #     print("\nPrevious Board: ",)

        # print("PREVIOUS BOARD:")
        # print(self.prev_board)
        # print(self.prev_board.fen())
        # print("CURRENT BOARD:")
        # print(self.board)
        # print(self.board.fen())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AI Chess Robot with Computer Vision")
    parser.add_argument("--difficulty", choices=["weenie_hut_junior", "easy", "medium", "hard"], 
                        default="medium", help="Chess AI difficulty (how far it looks ahead)")
    parser.add_argument("--color_scheme", choices=["r/t", "p/y", "o/b"], 
                        default="o/b", help="Red and teal (not working), pink and yellow, and orange and blue for chess piece colors")
    parser.add_argument("--show_cv", action="store_true", help="Show opencv images as processing occurs during game")
    parser.add_argument("--show_board", action="store_true", help="Show only the board (will show warped version)")
    parser.add_argument("--enable_castling", action="store_true", help="Enable castling")
    parser.add_argument("--mag_arm_enable", action="store_true", help="Use the magnetic arm")
    parser.add_argument("--show_cam", action="store_true", help="Show persistent camera view")
    parser.add_argument("--loop", action="store_true", help="Loop before cv (for taking test images)")
    parser.add_argument("--img_idx", help="Where to start indexing images (for naming them; default is 1 if not specified)")
    parser.add_argument("--test_img", help="If specified, will use said image in test_images folder rather than camera input")
    parser.add_argument("--save_img_as", help="If specified, will save image as given name in game_images")
    args = parser.parse_args()

    game = ChessGame(args.difficulty, args.color_scheme, args.show_cv, args.show_board, args.enable_castling, args.mag_arm_enable, args.show_cam, args.loop, args.img_idx, args.test_img, args.save_img_as)
    game.start_game()