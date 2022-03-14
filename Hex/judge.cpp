#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <queue>
#include "jsoncpp/json.h"
using namespace std;

const int BLACK = 1; //棋盘为黑
const int WHITE = -1; // 棋盘为白
const int EMPTY = 0; // 棋盘为空

const int CX[] = {-1, 0, 1, 0, -1, 1}; //搜索
const int CY[] = {0, -1, 0, 1, 1, -1};  //搜索
const int BOARD_LEN = 11; // 棋盘长度
int currentPlayer = BLACK; // 当前玩家
int board[BOARD_LEN][BOARD_LEN] = {0}; // 当前棋盘

const int firstMoveX = 1;
const int firstMoveY = 2; //第一手强制走这里，走别的地方直接判负

bool inBoard(int x, int y){
    return x >= 0 && y >=0 && x < BOARD_LEN && y < BOARD_LEN;
}


bool isWinHelper(bool* visited, int startX, int startY,int color)
{
    if (!inBoard(startX, startY))return false;
    int loc = startX * BOARD_LEN + startY;
    if(board[startX][startY]!=color)return false;
    if(visited[loc])return false;
    visited[loc] = true;
    if (color == BLACK && startX == BOARD_LEN - 1)return true;
    if (color == WHITE && startY == BOARD_LEN - 1)return true;
    for (int i = 0; i < 6; i++)
        if (isWinHelper(visited, startX + CX[i], startY + CY[i], color))return true;
    return false;
}

int checkWinner()
{
    bool visited[BOARD_LEN* BOARD_LEN] = { false };

    //check black first
    std::fill(visited, visited + BOARD_LEN * BOARD_LEN, false);
    for (int y = 0; y < BOARD_LEN ; y++)
    {
        if (board[0][y] == BLACK)
        {
            if (isWinHelper(visited, 0,y,BLACK))return BLACK;
        }
    }

    //check white
    std::fill(visited, visited + BOARD_LEN * BOARD_LEN, false);
    for (int x = 0; x <  BOARD_LEN ; x++)
    {
        if (board[x][0] == WHITE)
        {
            if (isWinHelper(visited, x,0,WHITE))return WHITE;
        }
    }
    return EMPTY;
}

bool ProcStep(int x, int y, int player, bool checkOnly = false)
{
    if (!inBoard(x, y) || board[x][y])
        return false;
    if (!checkOnly)
        board[x][y] = player;
    return true;
}


int main()
{
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input, output;
    reader.parse(str, input);
    input = input["log"];

    currentPlayer = 1; // 先手为黑
    if (input.size() == 0)
    {
        output["display"] = "";
        output["command"] = "request";
        output["content"]["0"]["x"] = -1;
        output["content"]["0"]["y"] = -1;
        output["content"]["0"]["forced_x"] = 1;
        output["content"]["0"]["forced_y"] = 2;
    }
    else
    {
        for (int i = 1; i < input.size(); i += 2)
        {
            bool isLast = i == input.size() - 1;
            Json::Value content;
            Json::Value display;
            if (currentPlayer == 1) // 0号玩家 / 黑方
            {
                Json::Value answer = input[i]["0"]["response"].isNull() ? input[i]["0"]["content"] : input[i]["0"]["response"];
                if (((answer.isString() &&
                      reader.parse(answer.asString(), content)) ||
                     (answer.isObject() &&
                      (content = answer, true))) &&
                    content["x"].isInt() && content["y"].isInt()) // 保证输入格式正确
                {
                    int currX = content["x"].asInt();
                    int currY = content["y"].asInt();
                    bool firstMoveIllegal = (i == 1 && (currX != firstMoveX || currY != firstMoveY));//第一手是否走在了指定位置，如果没有走在指定位置则判负
                    if ((!ProcStep(currX, currY, currentPlayer) && isLast)||firstMoveIllegal) // 不合法棋步！
                    {
                        stringstream ss;
                        ss << "INVALID_MOVE  (" << currX << "," << currY << ")";
                        if (inBoard(currX,currY)&&board[currX][currY]!=EMPTY)
                            ss << " is not empty";
                        else
                            ss << " first step is not (B, 3)";
                        string s; getline(ss, s);
                        output["display"]["err"] = s;
                        output["display"]["winner"] = 1;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 0;
                        output["content"]["1"] = 1;
                    }
                    else if (isLast) // 正常棋步
                    {
                        output["display"]["x"] = currX;
                        output["display"]["y"] = currY;
                        output["display"]["color"] = 0;
                        if (checkWinner()!=EMPTY) // 游戏结束
                        {
                            output["display"]["winner"] = 0;
                            output["command"] = "finish";
                            output["content"]["0"] = 1;
                            output["content"]["1"] = 0;
                        }
                        else
                        {
                            output["command"] = "request";
                            output["content"]["1"]["x"] = currX;
                            output["content"]["1"]["y"] = currY;
                        }
                    }
                }
                else if (isLast)
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["0"]["verdict"].asString();
                    output["display"]["winner"] = 1;
                    output["command"] = "finish"; // 判输
                    output["content"]["0"] = 0;
                    output["content"]["1"] = 1;
                }
            }
            else
            {
                Json::Value answer = input[i]["1"]["response"].isNull() ? input[i]["1"]["content"] : input[i]["1"]["response"];
                if (((answer.isString() &&
                      reader.parse(answer.asString(), content)) ||
                     (answer.isObject() &&
                      (content = answer, true))) &&
                    content["x"].isInt() && content["y"].isInt()) // 保证输入格式正确
                {
                    int currX = content["x"].asInt();
                    int currY = content["y"].asInt();
                    if (!ProcStep(currX, currY, currentPlayer) && isLast) // 不合法棋步！
                    {
                        stringstream ss;
                        ss << "INVALID_MOVE  (" << currX << "," << currY << ")";
                        if (inBoard(currX,currY)&&board[currX][currY]!=EMPTY)
                            ss << " is not empty";
                        string s; getline(ss, s);
                        output["display"]["err"] = s;
                        output["display"]["winner"] = 0;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 1;
                        output["content"]["1"] = 0;
                    }
                    else if (isLast) // 正常棋步
                    {
                        output["display"]["x"] = currX;
                        output["display"]["y"] = currY;
                        output["display"]["color"] = 1;
                        if (checkWinner()!=EMPTY) // 游戏结束
                        {
                            output["display"]["winner"] = 1;
                            output["command"] = "finish";
                            output["content"]["0"] = 0;
                            output["content"]["1"] = 1;
                        }
                        else
                        {
                            output["command"] = "request";
                            output["content"]["0"]["x"] = currX;
                            output["content"]["0"]["y"] = currY;
                        }
                    }
                }
                else if (isLast)
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["1"]["verdict"].asString();
                    output["display"]["winner"] = 0;
                    output["command"] = "finish"; // 判输
                    output["content"]["0"] = 1;
                    output["content"]["1"] = 0;
                }
            }
            currentPlayer *= -1;
        }
    }
    Json::FastWriter writer;
    cout << writer.write(output) << endl;
    return 0;
}