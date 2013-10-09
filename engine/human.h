#ifndef HUMAN_H
#define HUMAN_H
#pragma once

#include "common.h"

#include <vector>

template <typename S>
struct Human {
        void get_input(vector<string> &tokens) const {
                // get user input
                string s;
                std::getline(cin, s);

                // tokenize string
                istringstream iss(s);

                copy(istream_iterator<string>(iss),
                     istream_iterator<string>(),
                     back_inserter< vector<string> >(tokens));
        }

        bool valid_move(S &state, typename S::M &m) {
                typename S::ML ml;
                state.moves(ml);
                for (uint8_t i=0; i < ml.size(); ++i)
                        if (m == ml.move[i])
                                return true;
                return false;
        }

        void pretrain(size_t n) {}

        void next(Color c, S &state) {
                bool done=false, parsed=false;
                typename S::M m;
                while (!done) {
                        cout << "move> ";

                        vector<string> tokens;
                        get_input(tokens);

                        parsed = state.parse_move(m, tokens);
                        if (parsed) {
                                if (valid_move(state, m))
                                        done = true;
                                else cout << "! invalid move!" << endl;
                        }
                }
                state.move(m);
        }

        void set_param(float p) {}
};

#endif // HUMAN_H
