#include <iostream>
#include <sstream>
#include <string>
#include "jsoncpp/json.h"
using namespace std;

#define GRIDSIZE 8
#define JudgeBLACK 0
#define JudgeWHITE 1
#define GridBlack (-1)
#define GridWhite 1
#define GridEmpty 0

int gridInfo[GRIDSIZE][GRIDSIZE] = { GridEmpty }; // 先x后y,1为白，-1为黑，0为空位

bool inMap(int x, int y)
{
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return false;
    return true;
}

/*
 * O--------black--------------Y
 * |
 * |
 * |
 * |
 * |
 * |
 * X--------white-----------
 */

inline bool valid(int x0, int y0, int x1, int y1, int color)
{
    if (color == GridBlack){
        if (x0 + 1 != x1 || (y0 != y1 and y0 + 1 != y1 && y0 - 1 != y1)) {
            return false;
        }
    }
    if (color == GridWhite){
        if (x0 - 1 != x1 || (y0 != y1 && y0 + 1 != y1 && y0 - 1 != y1)) {
            return false;
        }
    }
    if (y1 == y0){
        if (gridInfo[x1][y1] != GridEmpty){
            return false;
        }
    } else {
        if (gridInfo[x1][y1] == color) {
            return false;
        }
    }
    return true;
}

bool procStep(int x0, int y0, int x1, int y1, int color)
{
    if ((!inMap(x0, y0)) || (!inMap(x1, y1)))
        return false;
    if (gridInfo[x0][y0] != color)
        return false;
    if (!valid(x0, y0, x1, y1, color))
        return false;
    gridInfo[x0][y0] = GridEmpty;
    gridInfo[x1][y1] = color;
    return true;
}

bool check_ifhasValidMove(int color)//判断待输入方是否存在合法走步
{
    int count = 0;
    for (int i = 0; i < GRIDSIZE; ++i) {
        for (int j = 0; j < GRIDSIZE; ++j) {
            if (gridInfo[i][j] == color){
                count++;
            }
        }
    }
    if (count == 0){
        return false;
    }
    if (color == GridWhite){
        for (int i = 0; i < GRIDSIZE; ++i) {
            if (gridInfo[GRIDSIZE - 1][i] == GridBlack){
                return false;
            }
        }
    }
    if (color == GridBlack){
        for (int i = 0; i < GRIDSIZE; ++i) {
            if (gridInfo[0][i] == GridWhite){
                return false;
            }
        }
    }
    return true;
}

int main()
{
    // 注意黑方为0号玩家,白方为1号玩家,每场对局黑方先手
    int currBotColor; // 正在等待输出结果的BOT

    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input, output;
    reader.parse(str, input);
    input = input["log"];

    for (int i = 0; i < GRIDSIZE; ++i) {
        gridInfo[0][i] = gridInfo[1][i] = GridBlack;
        gridInfo[GRIDSIZE - 2][i] = gridInfo[GRIDSIZE - 1][i] = GridWhite;
    }

    currBotColor = GridWhite;
    if (input.size() == 0)// 先手为黑(0),此时应得到输入
    {
        output["command"] = "request";
        output["content"]["1"]["x0"] = -1;
        output["content"]["1"]["y0"] = -1;
        output["content"]["1"]["x1"] = -1;
        output["content"]["1"]["y1"] = -1;
    }
    else
    {
        for (int i = 1; i < input.size(); i += 2)
        {
            bool isLast = i == input.size() - 1;
            Json::Value content;
            Json::Value display;
            if (currBotColor == GridBlack) // 0号玩家 / 黑方
            {
                Json::Value answer = input[i]["0"]["response"].isNull() ? input[i]["0"]["content"] : input[i]["0"]["response"];
                if (((answer.isString() &&
                      reader.parse(answer.asString(), content)) ||
                     (answer.isObject() &&
                      (content = answer, true))) &&
                    content["x0"].isInt() && content["y0"].isInt() &&
                    content["x1"].isInt() && content["y1"].isInt()) // 保证输入格式正确
                {
                    int x0 = content["x0"].asInt();
                    int y0 = content["y0"].asInt();
                    int x1 = content["x1"].asInt();
                    int y1 = content["y1"].asInt();
                    bool out_continue = procStep(x0, y0, x1, y1, GridBlack);
                    if (!out_continue && isLast) // 黑方不合法棋步！
                    {
                        output["display"]["err"] = "INVALIDMOVE";
                        output["display"]["winner"] = JudgeWHITE;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 0;
                        output["content"]["1"] = 1;
                    }
                    else if (isLast) // 最后一步
                    {
                        output["display"]["x0"] = x0;//传递给播放器的信息
                        output["display"]["y0"] = y0;
                        output["display"]["x1"] = x1;
                        output["display"]["y1"] = y1;
                        if (!check_ifhasValidMove(GridWhite)) // 待输入方无合法走步
                        {
                            output["display"]["winner"] = JudgeBLACK;
                            output["command"] = "finish";
                            output["content"]["0"] = 1;
                            output["content"]["1"] = 0;
                        }
                        else
                        {
                            output["command"] = "request";//给待输入方的信息
                            output["content"]["1"]["x0"] = x0;
                            output["content"]["1"]["y0"] = y0;
                            output["content"]["1"]["x1"] = x1;
                            output["content"]["1"]["y1"] = y1;
                        }
                    }
                }
                else//非法输入
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["0"]["verdict"].asString(); // 输出格式错误
                    output["display"]["winner"] = JudgeWHITE;
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
                    content["x0"].isInt() && content["y0"].isInt() &&
                    content["x1"].isInt() && content["y1"].isInt()) // 保证输入格式正确
                {
                    int x0 = content["x0"].asInt();
                    int y0 = content["y0"].asInt();
                    int x1 = content["x1"].asInt();
                    int y1 = content["y1"].asInt();
                    bool out_continue = procStep(x0, y0, x1, y1, GridWhite);
                    if (!out_continue && isLast) // 不合法棋步！
                    {
                        output["display"]["err"] = "INVALIDMOVE";
                        output["display"]["winner"] = JudgeBLACK;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 1;
                        output["content"]["1"] = 0;
                    }
                    else if (isLast) // 最后一步
                    {
                        output["display"]["x0"] = x0;//传递给播放器的信息
                        output["display"]["y0"] = y0;
                        output["display"]["x1"] = x1;
                        output["display"]["y1"] = y1;
                        if (!check_ifhasValidMove(GridBlack)) // 待输入方无合法走步
                        {
                            output["display"]["winner"] = JudgeWHITE;
                            output["command"] = "finish";
                            output["content"]["0"] = 0;
                            output["content"]["1"] = 1;
                        }
                        else
                        {
                            output["command"] = "request";//给待输入方的信息
                            output["content"]["0"]["x0"] = x0;
                            output["content"]["0"]["y0"] = y0;
                            output["content"]["0"]["x1"] = x1;
                            output["content"]["0"]["y1"] = y1;
                        }
                    }
                }
                else//非法输入
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["1"]["verdict"].asString(); // 输出格式错误
                    output["display"]["winner"] = JudgeBLACK;
                    output["command"] = "finish"; // 判输
                    output["content"]["0"] = 1;
                    output["content"]["1"] = 0;
                }
            }
            currBotColor = -currBotColor;
        }
    }
    Json::FastWriter writer;
    std::cout << writer.write(output) << endl;
}