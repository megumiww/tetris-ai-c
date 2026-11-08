#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// 游戏参数
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define DEATH_LINE 15

// 方块类型枚举
typedef enum {
    I_BLOCK, T_BLOCK, O_BLOCK, J_BLOCK, L_BLOCK, S_BLOCK, Z_BLOCK
} BlockType;

// 方块旋转状态
typedef enum {
    R0, R90, R180, R270
} Rotation;

// 方向枚举
typedef enum {
    LEFT, RIGHT
} Direction;

// 坐标结构
typedef struct {
    int x, y;
} Point;

// 方块结构
typedef struct {
    BlockType type;
    Rotation rotation;
    Point position;
} Block;

// 游戏板结构
typedef struct {
    bool cells[BOARD_HEIGHT][BOARD_WIDTH];
    int score;
} Board;

// 移动结构，存储操作序列
typedef struct {
    int x;              // 最终x位置
    Rotation rotation;  // 最终旋转状态
    double score;       // 评估分数
} Move;

// 方块形状定义 (相对坐标)
const Point SHAPES[7][4][4] = {
    // I_BLOCK
    {
        {{0,0}, {1,0}, {2,0}, {3,0}},  // 0°
        {{0,0}, {0,1}, {0,2}, {0,3}},  // 90°
        {{0,0}, {1,0}, {2,0}, {3,0}},  // 180°
        {{0,0}, {0,1}, {0,2}, {0,3}}   // 270°
    },
    // T_BLOCK
    {
        {{0,0}, {1,0}, {2,0}, {1,1}},  // 0°
        {{0,0}, {0,1}, {0,2}, {1,1}},  // 这才是90°
        {{1,0}, {0,1}, {1,1}, {2,1}},  // 180°
        {{1,0}, {0,1}, {1,1}, {1,2}},  // 这才是270°
    },
    // O_BLOCK
    {
        {{0,0}, {1,0}, {0,1}, {1,1}},  // 0°
        {{0,0}, {1,0}, {0,1}, {1,1}},  // 90°
        {{0,0}, {1,0}, {0,1}, {1,1}},  // 180°
        {{0,0}, {1,0}, {0,1}, {1,1}}   // 270°
    },
    // 这才是J_BLOCK
    {
        {{0,0}, {1,0}, {2,0}, {0,1}},  // 0°
        {{0,0}, {0,1}, {0,2}, {1,2}},  // 90°
        {{2,0}, {0,1}, {1,1}, {2,1}},  // 180°
        {{0,0}, {1,0}, {1,1}, {1,2}}   // 270°
    },
    // 这才是L_BLOCK
    {
        {{0,0}, {1,0}, {2,0}, {2,1}},  // 0°
        {{0,0}, {1,0}, {0,1}, {0,2}},  // 90°
        {{0,0}, {0,1}, {1,1}, {2,1}},  // 180°
        {{1,0}, {1,1}, {1,2}, {0,2}}   // 270°
    },
    // 这才是S_BLOCK
    {
        {{0,0}, {1,0}, {1,1}, {2,1}},  // 0°
        {{1,0}, {0,1}, {1,1}, {0,2}},  // 90°
        {{0,0}, {1,0}, {1,1}, {2,1}},  // 180°
        {{1,0}, {0,1}, {1,1}, {0,2}}   // 270°
    },
    // 这才是Z_BLOCK
    {
        {{1,0}, {2,0}, {0,1}, {1,1}},  // 0°
        {{0,0}, {0,1}, {1,1}, {1,2}},  // 90°
        {{1,0}, {2,0}, {0,1}, {1,1}},  // 180°
        {{0,0}, {0,1}, {1,1}, {1,2}}   // 270°
    }
};

// 方块的宽度
int getBlockWidth(BlockType type, Rotation rotation) {
    int minX = 99, maxX = -1;
    for (int i = 0; i < 4; i++) {
        int x = SHAPES[type][rotation][i].x;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
    }
    return maxX - minX + 1;
}

// 方块的高度
int getBlockHeight(BlockType type, Rotation rotation) {
    int minY = 99, maxY = -1;
    for (int i = 0; i < 4; i++) {
        int y = SHAPES[type][rotation][i].y;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
    return maxY - minY + 1;
}

// 初始化游戏板
void initBoard(Board *board) {
    memset(board->cells, 0, sizeof(board->cells));
    board->score = 0;
}

// 检查位置是否有效
bool isValidPosition(Block block, Board board) {
    for (int i = 0; i < 4; i++) {
        int x = block.position.x + SHAPES[block.type][block.rotation][i].x;
        int y = block.position.y + SHAPES[block.type][block.rotation][i].y;
        
        // 检查边界
        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
            return false;
        }
        
        // 检查碰撞
        if (y >= 0 && y < BOARD_HEIGHT && board.cells[y][x]) {
            return false;
        }
    }
    return true;
}

// 旋转方块, 这不是游戏, 不需要考虑撞墙, 嗯转就行了
void rotateBlock(Block *block, Board board, Rotation newRotation) {
    Rotation oldRotation = block->rotation;
    block->rotation = newRotation;
}

// 这些函数不涉及修改board, 就不用Board*指针了
// 下落方块
bool dropBlock(Block *block, Board board) {
    block->position.y--;
    if (!isValidPosition(*block, board)) {
        block->position.y++;
        return false;
    }
    return true;
}

// 下落到底部
void dropToBottom(Block *block, Board board) {
    while (dropBlock(block, board)) {}
}

// 将方块固定到游戏板上
void placeBlock(Block block, Board *board) {
    for (int i = 0; i < 4; i++) {
        int x = block.position.x + SHAPES[block.type][block.rotation][i].x;
        int y = block.position.y + SHAPES[block.type][block.rotation][i].y;
        if (y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH) {
            board->cells[y][x] = true;
        }
    }
}

// 调试用了一下
void printBoard(Board board) {
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            printf("%d", board.cells[y][x]);
        }
        printf("\n");
    }
}

// 清除完整的行并 不要计算分数
int clearLines(Board *board) {
    int linesCleared = 0;

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        bool lineIsFull = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (!board->cells[y][x]) {
                lineIsFull = false;
                break;
            }
        }

        if (lineIsFull) {
            linesCleared++;
            // 将上面的行下移
            for (int yy = y; yy < BOARD_HEIGHT - 1; yy++) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board->cells[yy][x] = board->cells[yy + 1][x];
                }
            }
            // 清空最上面的行
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board->cells[BOARD_HEIGHT - 1][x] = false;
            }
            // 由于行下移，需要重新检查当前行
            y--;
        }
    }
    
    return linesCleared;
}

// 根据消除行数给分
int scoreForLines(int lines) {
    if (lines == 1) return 100;
    if (lines == 2) return 300;
    if (lines == 3) return 500;
    if (lines == 4) return 800;

    return 0;
}

// 不用检查游戏是否结束

// 计算洞的数量
int countHoles(Board *board) {
    int holes = 0;
    
    for (int x = 0; x < BOARD_WIDTH; x++) {
        bool blockFound = false;
        for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
            if (board->cells[y][x]) {
                blockFound = true;
            } else if (blockFound) {
                holes++;
            }
        }
    }
    
    return holes;
}

// 计算高度差异
int calculateHeightDifference(Board *board) {
    int heights[BOARD_WIDTH] = {0};
    int minHeight = BOARD_HEIGHT;
    int maxHeight = 0;
    
    // 计算每列的高度
    for (int x = 0; x < BOARD_WIDTH; x++) {
        bool foundBlock = false;
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            if (board->cells[y][x]) {
                heights[x] = y + 1;
                foundBlock = true;
                if (heights[x] > maxHeight) maxHeight = heights[x];
                if (heights[x] < minHeight) minHeight = heights[x];
                break;
            }
        }
        // 如果这一列没有方块，高度为0
        if (!foundBlock) {
            heights[x] = 0;
            if (minHeight == BOARD_HEIGHT) minHeight = 0;
        }
    }
    
    return maxHeight - minHeight;
}

// 克隆游戏板
void cloneBoard(Board *source, Board *target) {
    memcpy(target->cells, source->cells, sizeof(source->cells));
    target->score = source->score;
}

// 评估移动
double evaluateMove(Board *board, Block block, int linesCleared) {
    double score = 0;
    
    // 奖励消行 - 给予更高的权重
    switch (linesCleared) {
        case 1: score += 100; break;
        case 2: score += 300; break;
        case 3: score += 500; break;
        case 4: score += 800; break;
        default: break;
    }
    
    // 惩罚洞洞
    int holes = countHoles(board);
    score -= holes * 30;
    
    // 惩罚高度差异
    int heightDiff = calculateHeightDifference(board);
    score -= heightDiff * 10;
    
    // 计算最大高度，越低越好
    int maxHeight = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
            if (board->cells[y][x]) {
                int height = y + 1;
                if (height > maxHeight) maxHeight = height;
                break;
            }
        }
    }
    score -= maxHeight * 5;
    
    return score;
}

// 寻找最佳移动
Move findBestMove(Board *board, Block block, BlockType nextBlockType) {
    Move bestMove = {0, R0, -9999999};
    Board tempBoard;
    
    // 尝试所有可能的旋转
    for (Rotation rotation = R0; rotation <= R270; rotation++) {
        Block testBlock = block;
        
        // 计算可能的水平位置范围
        int blockWidth = getBlockWidth(block.type, rotation);
        int blockHeight = getBlockHeight(block.type, rotation);
        int minX = 0;
        int maxX = BOARD_WIDTH - blockWidth;
        
        // 尝试所有可能的水平位置
        for (int x = minX; x <= maxX; x++) {
            testBlock = block;
            testBlock.rotation = rotation;
            testBlock.position.x = x;
            testBlock.position.y = BOARD_HEIGHT - blockHeight - 1;
            
            // 克隆游戏板进行模拟
            cloneBoard(board, &tempBoard);
            
            // 将方块下落到底
            dropToBottom(&testBlock, tempBoard);
            
            // 放置方块
            placeBlock(testBlock, &tempBoard);
            
            int linesCleared = clearLines(&tempBoard);
            
            // 评估这个移动
            double moveScore = evaluateMove(&tempBoard, testBlock, linesCleared);

            // printf("Attempt: %d %d\n", x, rotation);
            // printBoard(tempBoard);
            // printf("evaluate score: %d\n", moveScore);
            
            // 如果这是最佳移动，保存它
            if (moveScore > bestMove.score) {
                bestMove.x = x;
                bestMove.rotation = rotation;
                bestMove.score = moveScore;
            }
        }
    }
    
    return bestMove;
}

// 从输入解析方块类型
BlockType parseBlockType(char c) {
    switch (c) {
        case 'I': return I_BLOCK;
        case 'T': return T_BLOCK;
        case 'O': return O_BLOCK;
        case 'J': return J_BLOCK;
        case 'L': return L_BLOCK;
        case 'S': return S_BLOCK;
        case 'Z': return Z_BLOCK;
        default:  return I_BLOCK;  // 默认返回I型方块
    }
}

int main() {
    Board board;
    initBoard(&board);
    
    // 读取初始两个方块
    char blockChar, nextBlockChar;
    scanf(" %c %c", &blockChar, &nextBlockChar);
    
    BlockType currentType, nextType;
    bool gameEnded = false;
    
    while (!gameEnded) {
        // 解析当前方块和下一个方块
        currentType = parseBlockType(blockChar);
        nextType = parseBlockType(nextBlockChar);
        
        // 创建新的方块，设置初始位置
        Block block;
        block.type = currentType;
        block.rotation = R0;  // 初始旋转无意义
        block.position.x = BOARD_WIDTH / 2 - 2;  // 初始位置无意义
        block.position.y = BOARD_HEIGHT - 1;  // 初始高度无意义
        
        // 找到最佳移动
        Move bestMove = findBestMove(&board, block, nextType);
        
        // 输出最佳移动的旋转角度和x位置
        printf("%d %d\n", (int)bestMove.rotation, bestMove.x);
        fflush(stdout);  // 确保第一行输出立即发送
        
        // 更新方块位置到最佳位置
        block.rotation = bestMove.rotation;
        block.position.x = bestMove.x;
        block.position.y = BOARD_HEIGHT - getBlockHeight(currentType, bestMove.rotation) - 1;
        
        // 下落到底
        dropToBottom(&block, board);
        
        // 放置方块
        placeBlock(block, &board);

        // 清除完整的行并更新分数
        int lines = clearLines(&board);
        int points = scoreForLines(lines);

        board.score += points;

        // debug
        // printBoard(board);

        // 输出当前得分
        printf("%d\n", board.score);
        fflush(stdout);  // 确保第二行输出立即发送
        
        // 读取下一个方块信息
        char nextInput;
        scanf(" %c", &nextInput);
        
        // 检查是否结束游戏
        if (nextInput == 'X' || nextInput == 'E') {
            gameEnded = true;
        } else {
            // 移动到下一个方块
            blockChar = nextBlockChar;
            nextBlockChar = nextInput;
        }
    }
    
    return 0;
}