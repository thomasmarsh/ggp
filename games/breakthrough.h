/*
 *  breakthrough.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef BT_STATE_H
#define BT_STATE_H

#include "common.h"
#include "game.h"

#include <vector>
#include <stack>
#include <iostream>
#include <sstream>
#include <stdint.h>

namespace breakthrough {

typedef unsigned short BTPiece;
const BTPiece BT_EMPTY_CELL	= 0x00;  // 00
const BTPiece BT_WHITE_PAWN	= 0x01;  // 01
const BTPiece BT_BLACK_PAWN	= 0x02;  // 10
const BTPiece BT_CAPTURE_CELL	= 0x03;  // 11
const BTPiece BT_PIECE_MASK	= 0x03;  // 11

typedef int BTPos;
const BTPos BT_CHAR_OFFSET = 'a';

typedef uint64_t BTBoard;
struct BTStateID
{
	BTBoard white;
	BTBoard black;
	BTPiece turn;
};
typedef BTBoard BTKey;
typedef BTBoard BTMove;
const BTMove NullMove = 0;
typedef std::vector<BTMove> BTMoves;
typedef std::vector<BTStateID> History;

typedef unsigned short BTScore;
const BTScore BT_NOT_OVER	= 0x00;  // 00
const BTScore BT_WHITE_WINS	= 0x01;  // 01
const BTScore BT_BLACK_WINS	= 0x02;  // 10
const BTScore BT_DRAW		= 0x03;  // 11

class BTState
{
public:
        typedef BTMoves ML;
        typedef BTMove M;

	BTBoard whiteboard;
	BTBoard blackboard;
private:
	BTPiece turn;
	History history;
	void getLineMovesWhite(BTMoves& moves, BTMove& from);
	void getLineMovesBlack(BTMoves& moves, BTMove& from);
public:
	BTState();
	~BTState();
	
        void copy_from(const BTState &s) {
                whiteboard = s.whiteboard;
                blackboard = s.blackboard;
                turn = s.turn;
        }
	BTPiece getSquare(const BTPos& col, const BTPos& row);
	void setSquare(const BTPos& col, const BTPos& row, const BTPiece& piece);
	void clear();
	void reset();
	BTStateID getID();
	BTBoard getWhiteboard(){return whiteboard;};
	BTBoard getBlackboard(){return blackboard;};
	BTPiece getTurn(){return turn;};
	void setTurn(const BTPiece& piece){turn = piece;};
	
        void moves(ML &ml) { getMoves(ml); }
	void getMoves(BTMoves& moves);
	bool isPieceAt(BTBoard& board, BTPos& pos);
	void make(const BTMove& move);  // Returns true if a piece was captured;
        void move(M &m) { make(m); }
	void retract(const BTStateID& id);
	void syncState(const BTStateID& id);
	BTScore isTerminal();
	bool isCapture(const BTMove& move);
	
	
	bool strToMove(std::string strMove, BTMove& move); 
	bool setPosition(std::string pos);
	
	std::string toString();

        int score(bool b) { return 0; }
        bool random_move(ML &ml, M&m) {
                moves(ml);
                if (!ml.size()) 
                        return false;
                m = ml[random() % ml.size()];
                return true;
        }

        float result(Color c) {
                BTScore s = isTerminal();
                if (s == BT_BLACK_WINS) {
                        if (c == BLACK) return 1.0;
                        if (c == WHITE) return 0;
                        DIE("notreached");
                }
                if (s == BT_WHITE_WINS) {
                        if (c == BLACK) return 0;
                        if (c == WHITE) return 1.0;
                        DIE("notreached");
                }
                return 0.5;
        }

        Color winner() {
                switch (isTerminal()) {
                case BT_WHITE_WINS: return WHITE;
                case BT_BLACK_WINS: return BLACK;
                }
                return NONE;
        }

        Color current() {
                switch (getTurn()) {
                case BT_WHITE_PAWN: return WHITE;
                case BT_BLACK_PAWN: return BLACK;
                }
                return NONE;
        }

        void set_game_over() {
                turn = BT_DRAW;
        }

        bool game_over() { return isTerminal() > 0; }
        void print() {
                std::cout << toString() << std::endl;
        }
	
	static void getPieceName(const BTPiece& piece, std::ostream& istr)
	{
		if(piece == BT_WHITE_PAWN)
			istr << "White";
		else if(piece == BT_BLACK_PAWN)
			istr << "Black";
		else
			istr << "n/a";
	};
	
        void announce(const BTMove &move) {
                LOG(moveToString(move, current(), isCapture(move)));
        }
	static std::string moveToString(const BTMove& move, BTPiece turn, bool capture)
	{
		std::stringstream ss;
		BTMove mask = 1;
		BTMove temp = move;
		char firstcol = 'a';
		int firstrow = 0;
		char secondcol = 'a';
		int secondrow = 0;
		int i;
		for(i = 0 ; i < 64 ; i++)
		{
			if(temp & mask)
			{
				firstcol = (char)(i%8+BT_CHAR_OFFSET);
				firstrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		temp = temp >> 1;
		for(i+=1; i < 64 ; i++)
		{
			if(temp & mask)
			{
				secondcol = (char)(i%8+BT_CHAR_OFFSET); 
				secondrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		if(turn == BT_WHITE_PAWN)
		{
			ss << firstcol << firstrow;
			if(capture)
				ss << "x";
			else
				ss << "-";
			ss << secondcol << secondrow;
			
		}
		else
		{
			ss << secondcol << secondrow;
			if(capture)
				ss << "x";
			else
				ss << "-";
			ss << firstcol << firstrow;
		}
		return ss.str();
	};
	static std::string moveToString(const BTMove& move, BTState &state)
	{
		return moveToString(move, state.getTurn(), state.isCapture(move));
	}
	static std::string moveToPlayString(const BTMove& move, BTPiece turn)
	{
		std::stringstream ss;
		BTMove mask = 1;
		BTMove temp = move;
		char firstcol = 'a';
		int firstrow = 0;
		char secondcol = 'a';
		int secondrow = 0;
		int i;
		for(i = 0 ; i < 64 ; i++)
		{
			if(temp & mask)
			{
				firstcol = (char)(i%8+BT_CHAR_OFFSET);
				firstrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		temp = temp >> 1;
		for(i+=1; i < 64 ; i++)
		{
			if(temp & mask)
			{
				secondcol = (char)(i%8+BT_CHAR_OFFSET); 
				secondrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		if(turn == BT_WHITE_PAWN)
		{
			ss << firstcol << firstrow;
			ss << secondcol << secondrow;
		}
		else
		{
			ss << secondcol << secondrow;
			ss << firstcol << firstrow;
		}
		return ss.str();
	}
	static std::string boardToBitString(const BTBoard& board)
	{
		BTBoard temp = board;
		BTBoard mask = 1;
		std::stringstream ss;
		for(int n = 0 ; n < 63 ; n++)
		{
			if(temp & mask)
				ss << "1";
			else
				ss << "0";
			
			temp = temp >> 1;	
		}
		return ss.str();
	};
	static std::string moveToBitString(const BTMove& move)
	{
		return boardToBitString(move);
	}
};

} // namespace breakthrough

#endif // BT_STATE_H
