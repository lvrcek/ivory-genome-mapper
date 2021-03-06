// Copyright (c) 2021 Lovro Vrcek

#include "aligner.hpp"

#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>



namespace ivory {

int GlobalAlignment(
        const char* query, unsigned int query_len,
        const char* target, unsigned int target_len,
        int match,
        int mismatch,
        int gap,
        int gap_open,
        int gap_extend,
        std::string* cigar,
        unsigned int* target_begin,
        bool matrix_print) {
    int** matrix = new int*[query_len + 1];
    Direction** traceback = new Direction*[query_len + 1];
    for (int i = 0; i < query_len + 1; i++) {
        matrix[i] = new int[target_len + 1];
        traceback[i] = new Direction[target_len + 1];
    }

    matrix[0][0] = 0;
    traceback[0][0] = stop;

    for (int j = 1; j < target_len + 1; j++) {
        matrix[0][j] = gap * j;
        traceback[0][j] = left;
    }

    for (int i = 1; i < query_len  + 1; i++) {
        matrix[i][0] = gap * i;
        traceback[i][0] = up;
    }

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            int subs = matrix[i-1][j-1] +
                    ((query[i-1] == target[j-1]) ? match : mismatch);
            int ins, del;
            if (gap_open != 0 && gap_extend != 0) {
                ins = matrix[i][j-1] +
                        ((traceback[i][j-1] == left) ? gap_extend : gap_open);
                del = matrix[i-1][j] +
                        ((traceback[i-1][j] == up) ? gap_extend : gap_open);
            } else {
                ins = matrix[i][j-1] + gap;
                del = matrix[i-1][j] + gap;
            }

            std::vector<std::pair<int, Direction>> v;
            v.push_back(std::make_pair(subs, diag));
            v.push_back(std::make_pair(ins, left));
            v.push_back(std::make_pair(del, up));
            auto step = std::max_element(v.begin(), v.end());

            matrix[i][j] = step->first;
            traceback[i][j] = step->second;
        }
    }
    if (matrix_print) {
        printf("\n");
        PrintMatrix(matrix, query, query_len, target, target_len);
        PrintTraceback(traceback, query, query_len, target, target_len);
    }

    int score = matrix[query_len][target_len];
    int end_query = query_len;
    int end_target = target_len;

    if (cigar != nullptr)
        *cigar = GetCigar(traceback, end_query, end_target);
    if (target_begin != nullptr)
        *target_begin = GetTargetBegin(traceback, end_query, end_target);

    return score;
}

int LocalAlignment(
        const char* query, unsigned int query_len,
        const char* target, unsigned int target_len,
        int match,
        int mismatch,
        int gap,
        int gap_open,
        int gap_extend,
        std::string* cigar,
        unsigned int* target_begin,
        bool matrix_print) {
    int** matrix = new int*[query_len + 1];
    Direction** traceback = new Direction*[query_len + 1];

    for (int i = 0; i < query_len + 1; i++) {
        matrix[i] = new int[target_len + 1];
        traceback[i] = new Direction[target_len + 1];
    }

    matrix[0][0] = 0;
    traceback[0][0] = stop;

    for (int j = 1; j < target_len + 1; j++) {
        matrix[0][j] = 0;
        traceback[0][j] = stop;
    }

    for (int i = 1; i < query_len  + 1; i++) {
        matrix[i][0] = 0;
        traceback[i][0] = stop;
    }

    int score = 0;
    unsigned int end_query = 0, end_target = 0;

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            int subs = matrix[i-1][j-1] +
                    ((query[i-1] == target[j-1]) ? match : mismatch);
            int ins, del;
            if (gap_open != 0 && gap_extend != 0) {
                ins = matrix[i][j-1] +
                        ((traceback[i][j-1] == left) ? gap_extend : gap_open);
                del = matrix[i-1][j] +
                        ((traceback[i-1][j] == up) ? gap_extend : gap_open);
            } else {
                ins = matrix[i][j-1] + gap;
                del = matrix[i-1][j] + gap;
            }

            std::vector<std::pair<int, Direction>> v;
            v.push_back(std::make_pair(subs, diag));
            v.push_back(std::make_pair(ins, left));
            v.push_back(std::make_pair(del, up));
            v.push_back(std::make_pair(0, stop));
            auto step = std::max_element(v.begin(), v.end());

            matrix[i][j] = step->first;
            traceback[i][j] = step->second;

            if (matrix[i][j] > score) {
                score = matrix[i][j];
                end_query = i;
                end_target = j;
            }
        }
    }
    if (matrix_print) {
        printf("\n");
        PrintMatrix(matrix, query, query_len, target, target_len);
        PrintTraceback(traceback, query, query_len, target, target_len);
    }

    if (cigar != nullptr)
        *cigar = GetCigar(traceback, end_query, end_target);
    if (target_begin != nullptr)
        *target_begin = GetTargetBegin(traceback, end_query, end_target);

    return score;
}


int SemiGlobalAlignment(
        const char* query, unsigned int query_len,
        const char* target, unsigned int target_len,
        int match,
        int mismatch,
        int gap,
        int gap_open,
        int gap_extend,
        std::string* cigar,
        unsigned int* target_begin,
        bool matrix_print) {
    int** matrix = new int*[query_len + 1];
    Direction** traceback = new Direction*[query_len + 1];

    for (int i = 0; i < query_len + 1; i++) {
        matrix[i] = new int[target_len + 1];
        traceback[i] = new Direction[target_len + 1];
    }

    matrix[0][0] = 0;
    traceback[0][0] = stop;

    for (int j = 1; j < target_len + 1; j++) {
        matrix[0][j] = 0;
        traceback[0][j] = stop;
    }

    for (int i = 1; i < query_len  + 1; i++) {
        matrix[i][0] = 0;
        traceback[i][0] = stop;
    }

    int score = 0;
    unsigned int end_query = 0, end_target = 0;

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            int subs = matrix[i-1][j-1] +
                    ((query[i-1] == target[j-1]) ? match : mismatch);
            int ins, del;
            if (gap_open != 0 && gap_extend != 0) {
                ins = matrix[i][j-1] +
                        ((traceback[i][j-1] == left) ? gap_extend : gap_open);
                del = matrix[i-1][j] +
                        ((traceback[i-1][j] == up) ? gap_extend : gap_open);
            } else {
                ins = matrix[i][j-1] + gap;
                del = matrix[i-1][j] + gap;
            }

            std::vector<std::pair<int, Direction>> v;
            v.push_back(std::make_pair(subs, diag));
            v.push_back(std::make_pair(ins, left));
            v.push_back(std::make_pair(del, up));
            auto step = std::max_element(v.begin(), v.end());

            matrix[i][j] = step->first;
            traceback[i][j] = step->second;

            if ((i == query_len || j == target_len) &&  matrix[i][j] > score) {
                score = matrix[i][j];
                end_query = i;
                end_target = j;
            }
        }
    }
    if (matrix_print) {
        printf("\n");
        PrintMatrix(matrix, query, query_len, target, target_len);
        PrintTraceback(traceback, query, query_len, target, target_len);
    }

    if (cigar != nullptr)
        *cigar = GetCigar(traceback, end_query, end_target);
    if (target_begin != nullptr)
        *target_begin = GetTargetBegin(traceback, end_query, end_target);

    return score;
}

void PrintMatrix(
        int** matrix,
        const char * query, unsigned int query_len,
        const char* target, unsigned int target_len) {
    std::cout << "Score matrix:" << std::endl;
    printf("%4c%4c", ' ', ' ');
    for (int j = 0; j < target_len; j++)
        printf("%4c", target[j]);
    printf("\n");
    printf("%4c", ' ');
    for (int i = 0; i < query_len + 1; i++) {
        for (int j = 0; j < target_len + 1; j++)
            printf("%4d", matrix[i][j]);
        printf("\n");
        printf("%4c", query[i]);
    }
    printf("\n");
}

void PrintTraceback(
        Direction** traceback,
        const char * query, unsigned int query_len,
        const char* target, unsigned int target_len) {
    std::cout << "Traceback matrix:" <<std::endl;
    printf("%4c%4c", ' ', ' ');
    for (int j = 0; j < target_len; j++)
        printf("%4c", target[j]);
    printf("\n");
    printf("%4c", ' ');
    for (int i = 0; i < query_len + 1; i++) {
        for (int j = 0; j < target_len + 1; j++) {
            switch (traceback[i][j]) {
                case 0:
                    printf("%4c", 'U'); break;
                case 1:
                    printf("%4c", 'L'); break;
                case 2:
                    printf("%4c", 'D'); break;
                case 3:
                    printf("%4c", 'S'); break;
            }
        }
        printf("\n");
        printf("%4c", query[i]);
    }
    printf("\n");
}

std::string GetCigar(Direction** traceback, unsigned int end_query,
                     unsigned int end_target) {
    std::string cigar = "";
    int i = end_query;
    int j = end_target;
    while (true) {
        if (traceback[i][j] == diag) {
            cigar += 'M';
            i--;
            j--;
        } else if (traceback[i][j] == left) {
            cigar += 'I';
            j--;
        } else if (traceback[i][j] == up) {
            cigar += 'D';
            i--;
        } else {
            break;
        }
    }
    int count = 1;
    std::reverse(cigar.begin(), cigar.end());
    std::string new_cigar = "";

    for (int i = 1; i < cigar.size(); i++) {
        if (cigar[i] == cigar[i-1]) {
            count++;
        } else {
            new_cigar += std::to_string(count) + cigar[i-1];
            count = 1;
        }
    }
    new_cigar += std::to_string(count) + cigar[cigar.size()-1];

    return new_cigar;
}

unsigned int GetTargetBegin(Direction** traceback, unsigned int end_query,
                            unsigned int end_target) {
    int i = end_query;
    int j = end_target;
    while (true) {
        if (traceback[i][j] == diag) {
            i--;
            j--;
        } else if (traceback[i][j] == left) {
            j--;
        } else if (traceback[i][j] == up) {
            i--;
        } else {
            break;
        }
    }
    return j;
}

int Align(
        const char* query, unsigned int query_len,
        const char* target, unsigned int target_len,
        AlignmentType type,
        int match,
        int mismatch,
        int gap,
        int gap_open,
        int gap_extend,
        std::string* cigar,
        unsigned int* target_begin,
        bool matrix_print) {
    int alignment_score;
    switch (type) {
        case global:
            alignment_score = GlobalAlignment(
                    query, query_len,
                    target, target_len,
                    match, mismatch, gap,
                    gap_open, gap_extend,
                    cigar, target_begin,
                    matrix_print);
            break;
        case local:
            alignment_score = LocalAlignment(
                    query, query_len,
                    target, target_len,
                    match, mismatch, gap,
                    gap_open, gap_extend,
                    cigar, target_begin,
                    matrix_print);
            break;
        case semiglobal:
            alignment_score = SemiGlobalAlignment(
                    query, query_len,
                    target, target_len,
                    match, mismatch, gap,
                    gap_open, gap_extend,
                    cigar, target_begin,
                    matrix_print);
            break;
    }
    return alignment_score;
}

}  // namespace ivory
