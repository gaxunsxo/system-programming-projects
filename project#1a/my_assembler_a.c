/**
 * @file my_assembler_20221846.c
 * @date 2024-04-29
 * @version 0.1.0
 *
 * @brief SIC/XE 소스코드를 object code로 변환하는 프로그램
 *
 * @details
 * SIC/XE 소스코드를 해당 머신에서 동작하도록 object code로 변환하는
 * 프로그램이다. 파일 내에서 사용되는 문자열 "00000000"에는 자신의 학번을
 * 기입한다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* 파일명의 "00000000"은 자신의 학번으로 변경할 것 */
#include "my_assembler_20221846-1.h"

#define HEADER_RECORD 'H'
#define TEXT_RECORD 'T'
#define END_RECORD 'E'
#define DEFINE_RECORD 'D'
#define REFERENCE_RECORD 'R'
#define MODIFY_RECORD 'M'

void print_operands(FILE* fp, const token* tok);
static int add_inst_to_table(inst *inst_table[], int *inst_table_length,
                             const char *buffer);
void init_token(token *tok);
static int token_operand_parsing(const char *operand_input,
                                 int operand_input_length, char *operand[]);
void add_symbol(symbol **sym_table, int *sym_count, const char *name, int addr);
void add_literal(literal **lit_table, int *lit_count, const char *lit, int addr);
int search_symtab(const char *str, const symbol *symbol_table[], int symbol_table_length);
int search_littab(const char *str, const literal *literal_table[], int literal_table_length);
void format_object_code(char *buffer, token *tok, int opcode, int nixbpe, int disp, int format);
int calculate_nixbpe(const char *operand, const char *operator, int format);
int calculate_target_address(const char *operand, int locctr, int format,
                             const symbol *symbol_table[], int symbol_table_length, 
                             const literal *literal_table[], int literal_table_length);
void add_record(object_code *obj_code, char record_type, int section_index, const char *data);

/**
 * @brief 사용자로부터 SIC/XE 소스코드를 받아서 object code를 출력한다.
 *
 * @details
 * 사용자로부터 SIC/XE 소스코드를 받아서 object code를 출력한다. 특별한 사유가
 * 없는 한 변경하지 말 것.
 */
int main(int argc, char **argv) {
    /** SIC/XE 머신의 instruction 정보를 저장하는 테이블 */
    inst *inst_table[MAX_INST_TABLE_LENGTH];
    int inst_table_length;

    /** SIC/XE 소스코드를 저장하는 테이블 */
    char *input[MAX_INPUT_LINES];
    int input_length;

    /** 소스코드의 각 라인을 토큰 전환하여 저장하는 테이블 */
    token *tokens[MAX_INPUT_LINES];
    int tokens_length;

    /** 소스코드 내의 심볼을 저장하는 테이블 */
    symbol *symbol_table[MAX_TABLE_LENGTH];
    int symbol_table_length;

    /** 소스코드 내의 리터럴을 저장하는 테이블 */
    literal *literal_table[MAX_TABLE_LENGTH];
    int literal_table_length;

    /** 오브젝트 코드를 저장하는 변수 */
    object_code *obj_code = (object_code *)malloc(sizeof(object_code));

    int err = 0;

    if ((err = init_inst_table(inst_table, &inst_table_length,
                               "inst_table.txt")) < 0) {
        fprintf(stderr,
                "init_inst_table: 기계어 목록 초기화에 실패했습니다. "
                "(error_code: %d)\n",
                err);
        return -1;
    }

    if ((err = init_input(input, &input_length, "input.txt")) < 0) {
        fprintf(stderr,
                "init_input: 소스코드 입력에 실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    if ((err = assem_pass1((const inst **)inst_table, inst_table_length,
                           (const char **)input, input_length, tokens,
                           &tokens_length, symbol_table, &symbol_table_length,
                           literal_table, &literal_table_length)) < 0) {
        fprintf(stderr,
                "assem_pass1: 패스1 과정에서 실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    /** 프로젝트1에서는 불필요함 */
    /*
    if ((err = make_opcode_output("output_opcode.txt", (const token **)tokens,
                                  tokens_length, (const inst **)inst_table,
                                  inst_table_length)) < 0) {
        fprintf(stderr,
                "make_opcode_output: opcode 파일 출력 과정에서 실패했습니다. "
                "(error_code: %d)\n",
                err);
        return -1;
    }
    */

    if ((err = make_symbol_table_output("output_symtab.txt",
                                        (const symbol **)symbol_table,
                                        symbol_table_length)) < 0) {
        fprintf(stderr,
                "make_symbol_table_output: 심볼테이블 파일 출력 과정에서 "
                "실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    if ((err = make_literal_table_output("output_littab.txt",
                                         (const literal **)literal_table,
                                         literal_table_length)) < 0) {
        fprintf(stderr,
                "make_literal_table_output: 리터럴테이블 파일 출력 과정에서 "
                "실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    if ((err = assem_pass2((const token **)tokens, tokens_length,
                           (const inst **)inst_table, inst_table_length,
                           (const symbol **)symbol_table, symbol_table_length,
                           (const literal **)literal_table,
                           literal_table_length, obj_code)) < 0) {
        fprintf(stderr,
                "assem_pass2: 패스2 과정에서 실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    if ((err = make_objectcode_output("output_objectcode.txt",
                                      (const object_code *)obj_code)) < 0) {
        fprintf(stderr,
                "make_objectcode_output: 오브젝트코드 파일 출력 과정에서 "
                "실패했습니다. (error_code: %d)\n",
                err);
        return -1;
    }

    return 0;
}

/**
 * @brief 기계어 목록 파일(inst_table.txt)을 읽어 기계어 목록
 * 테이블(inst_table)을 생성한다.
 *
 * @param inst_table 기계어 목록 테이블의 시작 주소
 * @param inst_table_length 기계어 목록 테이블의 길이를 저장하는 변수 주소
 * @param inst_table_dir 기계어 목록 파일 경로
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 기계어 목록 파일(inst_table.txt)을 읽어 기계어 목록 테이블(inst_table)을
 * 생성한다. 기계어 목록 파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *    ==============================================================
 *           | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | \n |
 *    ==============================================================
 */
int init_inst_table(inst *inst_table[], int *inst_table_length,
                    const char *inst_table_dir) {
    /* add your code */
    FILE *fp;
    int err;

    char buffer[20];

    /* add your code here */
    // 파일 열기 
    fp = fopen(inst_table_dir, "r");
    if (fp == NULL) {
        fprintf(stderr, "기계 코드목록 파일 열기 실패");
        return -1;
    }

    // 파일을 읽어 기계어 목록 테이블에 저장
    while(!feof(fp)) {
        fgets(buffer, 20, fp);
        err = add_inst_to_table(inst_table, inst_table_length, buffer);
        if (err != 0) {
            fclose(fp);
            return err;
        }
    } 

    /* // 테스트 코드
    printf("%d\n", *inst_table_length);
    for (int i=0; i<*inst_table_length; i++) {
        printf("%s %d %X %d\n", inst_table[i]->str, inst_table[i]->format, inst_table[i]->op, inst_table[i]->ops);
    } */

    return err;
    
}

/**
 * @brief inst_table.txt의 라인 하나를 입력으로 받아, 해당하는 instruction
 * 정보를 inst_table에 저장함.
 */
static int add_inst_to_table(inst *inst_table[], int *inst_table_length, 
                             const char *buffer) {
    char name[10];
    int format;
    char op[10];
    int ops;

    if (*inst_table_length == MAX_INST_TABLE_LENGTH) return -1;

    sscanf(buffer, "%s %d %s %d\n", name, &format, op, &ops);

    if ((inst_table[*inst_table_length] = (inst*)malloc(sizeof(inst))) == NULL) 
        return -1;

    memcpy(inst_table[*inst_table_length]->str, name, 9);
    inst_table[*inst_table_length]->str[9] = '\0';

    inst_table[*inst_table_length]->format = format;

    inst_table[*inst_table_length]->op = (char)strtol(op, NULL, 16);

    inst_table[*inst_table_length]->ops = ops;

    ++(*inst_table_length);  

    return 0;
}

/**
 * @brief SIC/XE 소스코드 파일(input.txt)을 읽어 소스코드 테이블(input)을
 * 생성한다.
 *
 * @param input 소스코드 테이블의 시작 주소
 * @param input_length 소스코드 테이블의 길이를 저장하는 변수 주소
 * @param input_dir 소스코드 파일 경로
 * @return 오류 코드 (정상 종료 = 0)
 */
int init_input(char *input[], int *input_length, const char *input_dir) {
    /* add your code */
    FILE* fp;
    int err;
    int line_count = 0;


    // 파일 열기
    fp = fopen(input_dir, "r");
    if (fp == NULL) {
        perror("소스파일 열기 실패");
        return -1;
    }

    // 입력 길이 초기화
    *input_length = 0;

    // 파일을 라인 단위로 읽어 소스코드 테이블에 저장
    char line[MAX_INPUT_LINES];
    while (fgets(line, MAX_INPUT_LINES, fp) != NULL) {
        /* // 라인의 끝에 개행 문자 제거
        if (line[strlen(line) - 1] == '\n'){
            line[strlen(line) - 1] == '\0';
        } */

        // 소스코드 테이블에 라인 저장
        input[line_count] = strdup(line);
        if (input[line_count] == NULL) {
            fprintf(stderr, "메모리 할당 오류\n");
            fclose(fp);
            return -2; // 메모리 할당 실패 
        }

        line_count++;
    }

    // 입력 라인 수 설정
    *input_length = line_count;

    fclose(fp);

    return err;
}

// 토큰을 초기화하는 함수
void init_token(token *tok) {
    tok->label = NULL;
    tok->operator = NULL;
    for (int j = 0; j < MAX_OPERAND_PER_INST; j++) {
        tok->operand[j] = NULL;
    }
    tok->comment = NULL;
}

// 심볼 테이블에 항목을 추가하는 함수
void add_symbol(symbol **sym_table, int *sym_count, const char *name, int addr) {
    // 심볼 객체에 메모리 할당
    sym_table[*sym_count] = (symbol *)malloc(sizeof(symbol));
    if (sym_table[*sym_count] == NULL) {
        fprintf(stderr, "Memory allocation failed for new symbol\n");
        return;  // 메모리 할당 실패 시, 함수를 바로 종료
    }

    // 심볼 이름과 주소를 할당
    strcpy(sym_table[*sym_count]->name, name);
    sym_table[*sym_count]->addr = addr;
    (*sym_count)++;
}

// 리터럴 테이블에 항목을 추가하는 함수
void add_literal(literal **lit_table, int *lit_count, const char *lit, int addr) {
    // 리터럴 객체에 메모리 할당
    lit_table[*lit_count] = (literal *)malloc(sizeof(literal));
    if (lit_table[*lit_count] == NULL) {
        fprintf(stderr, "Memory allocation failed for new symbol\n");
        return;  // 메모리 할당 실패 시, 함수를 바로 종료
    }

    // 리터럴 이름과 주소를 할당
    strcpy(lit_table[*lit_count]->literal, lit);
    lit_table[*lit_count]->addr = addr;
    (*lit_count)++;
}

/**
 * @brief 어셈블리 코드을 위한 패스 1 과정을 수행한다.
 *
 * @param inst_table 기계어 목록 테이블의 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @param input 소스코드 테이블의 주소
 * @param input_length 소스코드 테이블의 길이
 * @param tokens 토큰 테이블의 시작 주소
 * @param tokens_length 토큰 테이블의 길이를 저장하는 변수 주소
 * @param symbol_table 심볼 테이블의 시작 주소
 * @param symbol_table_length 심볼 테이블의 길이를 저장하는 변수 주소
 * @param literal_table 리터럴 테이블의 시작 주소
 * @param literal_table_length 리터럴 테이블의 길이를 저장하는 변수 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 어셈블리 코드를 위한 패스1 과정을 수행하는 함수이다. 패스 1에서는 프로그램
 * 소스를 스캔하여 해당하는 토큰 단위로 분리하여 프로그램 라인별 토큰 테이블을
 * 생성한다. 토큰 테이블은 token_parsing 함수를 호출하여 설정하여야 한다. 또한,
 * assem_pass2 과정에서 사용하기 위한 심볼 테이블 및 리터럴 테이블을 생성한다.
 */
int assem_pass1(const inst *inst_table[], int inst_table_length,
                const char *input[], int input_length, token *tokens[],
                int *tokens_length, symbol *symbol_table[],
                int *symbol_table_length, literal *literal_table[],
                int *literal_table_length) {
    /* add your code */
    int err;
    char *line; // 임시 버퍼
    int locctr = 0;
    int num_tokens = 0;
    int num_symbols = 0;
    int num_literals = 0;

    // 심볼 테이블, 리터럴 테이블 동적 할당
    *symbol_table = (symbol*)calloc(input_length, sizeof(symbol));
    *literal_table = (literal*)calloc(input_length, sizeof(literal));

    // 입력된 각 라인에 대해 토큰을 생성하고 PASS1 수행
    for (int i = 0; i < input_length; ++i) {
        line = strdup(input[i]); // 입력 라인 복사
        if (line == NULL) {
            fprintf(stderr, "Memory allocation failed for input line at index %d\n", i);
            return -1;  // 메모리 할당 실패 처리
        }

        // 개행 문자 제거
        size_t line_length = strlen(line);
        if (line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0'; // 개행 문자 제거
        }

        tokens[i] = (token *)malloc(sizeof(token));
        if (tokens[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for token at index %d\n", i);
            return -1;  // Ensure all previous tokens are freed before return if necessary
        }

        init_token(tokens[i]);

        err = token_parsing(input[i], tokens[i], inst_table, inst_table_length);
        if (err != 0) {
            fprintf(stderr, "Token parsing failed at index %d with error %d\n", i, err);
            // Free allocated memory for current and previous tokens if needed
            return err;
        }
        num_tokens++;

        // 심볼 테이블 처리
        if (tokens[i]->label != NULL) {
            if (strcmp(tokens[i]->operator, "CSECT") == 0) {
                add_symbol(symbol_table, &num_symbols, tokens[i]->label, 0);
            } else {
                add_symbol(symbol_table, &num_symbols, tokens[i]->label, locctr);
            }
        }

        // 리터럴 테이블 처리 
        for (int j = 0; j < MAX_OPERAND_PER_INST; j++) {
            if (tokens[i]->operand[j] != NULL && tokens[i]->operand[j][0] == '=') {
                bool literal_exists = false;
                for (int k = 0; k < num_literals; k++) {
                    if (strcmp(literal_table[k]->literal, tokens[i]->operand[j]) == 0) {
                        literal_exists = true;
                        break;
                    }
                }
                if (!literal_exists) {
                    add_literal(literal_table, &num_literals, tokens[i]->operand[j], -1);
                }
            }
        }
        
        // 연산자(operator)에 따라 locctr을 조절
        if (input[i][0] == '.') {
            // 주석 라인은 토큰을 생성하지 않고 건너뜀
            continue;
        } else if (strcmp(tokens[i]->operator, "START") == 0) {
            if (tokens[i]->operand[0] != NULL) {
                locctr = strtol(tokens[i]->operand[0], NULL, 16); // 시작 주소 설정
            }
            // START 지시어의 경우에는 locctr만 변경하고 토큰의 개수를 증가시키지 않음
            continue;
        } else if (strcmp(tokens[i]->operator, "CSECT") == 0) {
            // Control section의 시작을 나타내는 경우, locctr을 초기화
            locctr = 0;
            continue;
        } else if (strcmp(tokens[i]->operator, "RESB") == 0) {
            // RESB 지시어의 경우, operand 값에 따라 locctr을 증가
            if (tokens[i]->operand[0] != NULL) {
                int decimal_value = atoi(tokens[i]->operand[0]); // 문자열을 정수로 변환
                char hex_string[5]; // 16진수 문자열을 저장할 공간 (4자리 + 널 문자)
                
                // 정수를 16진수 문자열로 변환하여 hex_string에 저장
                sprintf(hex_string, "%04X", decimal_value);

                // hex_string에 저장된 16진수를 다시 정수로 변환하여 locctr에 더함
                locctr += strtol(hex_string, NULL, 16);
            }
        } else if (strcmp(tokens[i]->operator, "RESW") == 0) {
            // RESW 지시어의 경우, operand 값에 따라 locctr을 증가
            if (tokens[i]->operand[0] != NULL) {
                locctr += 3 * strtol(tokens[i]->operand[0], NULL, 16); // operand 값에 따라 locctr 증가
            }
        } else if (strcmp(tokens[i]->operator, "EXTDEF") == 0 || strcmp(tokens[i]->operator, "EXTREF") == 0) {
            // EXTDEF 또는 EXTREF 지시어의 경우, locctr을 변경하지 않음
            continue;
        } else if (strcmp(tokens[i]->operator, "LTORG") == 0) {
            // LTORG 지시어의 경우, 리터럴 풀의 주소값을 계산하고 리터럴 테이블에 주소값을 할당
            for (int k = 0; k < num_literals; k++) {
                // 리터럴의 주소값을 현재 locctr로 재할당
                if (literal_table[k]->addr == -1) {
                    literal_table[k]->addr = locctr;
                    locctr += 3;
                }
            }
        } else if (strcmp(tokens[i]->operator, "EQU") == 0) {
            // EQU 지시어에서 피연산자가 없는 경우
            if (tokens[i]->operand[0] == NULL) {
                fprintf(stderr, "EQU 지시어에 피연산자가 없습니다.\n");
                return -1;
            }

            // EQU 지시어에서 피연산자가 현재 심볼인 경우
            if (strcmp(tokens[i]->operand[0], "*") == 0) {
                // 이미 심볼 테이블에 존재하는 경우 예외 처리
                int symbol_index;
                for (symbol_index = 0; symbol_index < num_symbols; symbol_index++) {
                    if (strcmp(tokens[i]->label, symbol_table[symbol_index]->name) == 0) {
                        symbol_table[symbol_index]->addr = locctr;
                        break;
                    }
                }
                if (symbol_index == num_symbols) {
                    symbol_table[num_symbols]->addr = locctr;
                    strcpy(symbol_table[num_symbols]->name, tokens[i]->label);
                    num_symbols++;
                }
            } else {
                // 피연산자가 심볼들의 연산인 경우
                int operand_expr;
                
                // BUFEND와 BUFFER의 주소값을 탐색
                int bufend_addr = -1;
                int buffer_addr = -1;
                for (int j = 0; j < num_symbols; j++) {
                    if (strcmp(symbol_table[j]->name, "BUFEND") == 0) {
                        bufend_addr = symbol_table[j]->addr;
                    } else if (strcmp(symbol_table[j]->name, "BUFFER") == 0) {
                        buffer_addr = symbol_table[j]->addr;
                    }
                }
                
                // BUFEND와 BUFFER의 주소값을 찾은 경우 연산을 수행
                if (bufend_addr != -1 && buffer_addr != -1) {
                    operand_expr = bufend_addr - buffer_addr;
                } else {
                    fprintf(stderr, "BUFEND 또는 BUFFER 심볼의 주소를 찾을 수 없습니다.\n");
                    return -1;
                }

                // 주소값만 바꿔줌
                int symbol_index;
                for (symbol_index = 0; symbol_index < num_symbols; symbol_index++) {
                    if (strcmp(tokens[i]->label, symbol_table[symbol_index]->name) == 0) {
                        symbol_table[symbol_index]->addr = operand_expr;
                        break;
                    }
                }   
            }
            continue;
        } else if (strcmp(tokens[i]->operator, "BYTE") == 0) {
            // BYTE 지시어의 경우, operand 값을 기반으로 할당할 메모리 크기를 계산하여 locctr을 증가
            if (tokens[i]->operand[0] != NULL) {
                int operand_length = strlen(tokens[i]->operand[0]);
                if (tokens[i]->operand[0][0] == 'X') {
                    // X로 시작하는 경우 두 글자당 한 바이트를 할당
                    locctr += (operand_length - 3) / 2;
                } else if (tokens[i]->operand[0][0] == 'C') {
                    // C로 시작하는 경우 한 글자당 한 바이트를 할당
                    locctr += operand_length - 3;
                }
            }
        } else if (strcmp(tokens[i]->operator, "WORD") == 0) {
            // WORD 지시어의 경우, 3바이트(한 워드)를 할당
            locctr += 3;
        } else if (strcmp(tokens[i]->operator, "END") == 0) {
            // END 지시어의 경우, 프로그램 실행을 시작하는 주소를 설정
            // LTORG 문을 만나지 못한 리터럴의 주소값을 설정
            for (int k = 0; k < num_literals; k++) {
                if (literal_table[k]->addr == -1) {
                    literal_table[k]->addr = locctr;
                }
            }
            // END 지시어는 프로그램의 종료를 의미하므로 더 이상 처리할 필요가 없음
            break;
        } else {
            // 그 외의 경우, 명령어 형식에 따라 locctr을 증가
            if (tokens[i]->operator[0] == '+') {
                locctr += 4; // 4형식 명령어
            } else {
                int opcode_index = search_opcode(tokens[i]->operator, inst_table, inst_table_length);
                if (opcode_index != -1) {
                    locctr += inst_table[opcode_index]->format;
                } else if (strcmp(tokens[i]->operator, "LTORG") == 0) {
                    locctr = 0;
                } else {
                    printf("%s",tokens[i]->operator);
                    return -1;
                }
            }
        }
    }

    *tokens_length = num_tokens;
    *symbol_table_length = num_symbols;
    *literal_table_length = num_literals;

    return 0;       
}


/**
 * @brief 한 줄의 소스코드를 파싱하여 토큰에 저장한다.
 *
 * @param input 파싱할 소스코드 문자열
 * @param tok 결과를 저장할 토큰 구조체 주소
 * @param inst_table 기계어 목록 테이블의 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @return 오류 코드 (정상 종료 = 0)
 */
int token_parsing(const char *input, token *tok, const inst *inst_table[],
                  int inst_table_length) {
    /* add your code */
    int input_length = strlen(input);

    tok->label = NULL;
    tok->operator= NULL;
    tok->operand[0] = NULL;
    tok->operand[1] = NULL;
    tok->operand[2] = NULL;
    tok->comment = NULL;

    if (input[0] == '.') {
        if ((tok->comment = (char *)malloc(input_length)) == NULL)
            return -1;
        sscanf(input + 1, " %[^\0]", tok->comment);
        tok->comment[input_length] = '\0';
    } else {
        int token_cnt = 0;
        for (int st = 0; st < input_length && token_cnt < 3; ++st) {
            int end = st;
            for (; input[end] != '\t' && input[end] != '\0'; ++end)
                ;

            switch (token_cnt) {
                case 0:
                    if (st < end) {
                        if ((tok->label = (char *)malloc(end - st + 1)) == NULL)
                            return -1;
                        memcpy(tok->label, input + st, end - st);
                        tok->label[end - st] = '\0';
                    }
                    break;

                case 1:
                    if (st < end) {
                        if ((tok->operator=(char *) malloc(end - st + 1)) ==
                            NULL)
                            return -1;
                        memcpy(tok->operator, input + st, end - st);
                        tok->operator[end - st] = '\0';
                    }
                    break;

                case 2:
                    if (st < end) {
                        int err;
                        if ((err = token_operand_parsing(input + st, end - st,
                                                         tok->operand)) != 0)
                            return err;
                    }

                    st = end + 1;
                    end = input_length;
                    if (st < end) {
                        if ((tok->comment = (char *)malloc(end - st + 1)) ==
                            NULL)
                            return -1;
                        memcpy(tok->comment, input + st, end - st);
                        tok->comment[end - st] = '\0';
                    }
            }

            ++token_cnt;
            st = end;
        }
    }

    return 0;
}

/**
 * @brief 피연산자 문자열을 파싱하여 operand에 저장함. 문자열은 \0으로 끝나지
 * 않을 수 있기에, operand_input_length로 문자열의 길이를 전달해야 함.
 */
static int token_operand_parsing(const char *operand_input,
                                 int operand_input_length, char *operand[]) {
    int operand_cnt = 0;
    int st = 0;
    while (st < operand_input_length && operand_cnt < 3) {
        while (st < operand_input_length && (operand_input[st] == ' ' || operand_input[st] == '\t' || operand_input[st] == '\n')) {
            st++;  // 시작 지점에서 화이트스페이스 및 개행문자 건너뛰기
        }

        int end = st;
        for (; end < operand_input_length && operand_input[end] != ',' && operand_input[end] != '\t' && 
               operand_input[end] != '\0' && operand_input[end] != '\n'; ++end)
            ;

        if (end > st) {  // 유효한 피연산자가 있는 경우에만 메모리 할당
            operand[operand_cnt] = (char *)malloc(end - st + 1);
            if (operand[operand_cnt] == NULL) {
                return -1;  // 메모리 할당 실패
            }
            memcpy(operand[operand_cnt], operand_input + st, end - st);
            operand[operand_cnt][end - st] = '\0';  // 피연산자 끝에 널 문자 추가
            ++operand_cnt;
        }

        st = end + 1;  // 쉼표 다음 위치로 시작 지점 업데이트
    }

    return 0;
}

/**
 * @brief 기계어 목록 테이블에서 특정 기계어를 검색하여, 해당 기계어가 위치한
 * 인덱스를 반환한다.
 *
 * @param str 검색할 기계어 문자열
 * @param inst_table 기계어 목록 테이블 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @return 기계어의 인덱스 (해당 기계어가 없는 경우 -1)
 *
 * @details
 * 기계어 목록 테이블에서 특정 기계어를 검색하여, 해당 기계어가 위치한 인덱스를
 * 반환한다. '+JSUB'와 같은 문자열에 대한 처리는 자유롭게 처리한다.
 */
int search_opcode(const char *str, const inst *inst_table[],
                  int inst_table_length) {
    /* add your code */
    if (str == NULL) {
        fprintf(stderr, "입력된 문자열이 NULL을 가리킵니다.\n");
        return -1;
    }
     
    // 입력된 기계어 코드와 기계어 테이블을 비교하여 인덱스를 찾는다.
    if (str[0] == '+') 
        return search_opcode(str + 1, inst_table, inst_table_length);
    
    for (int i=0; i < inst_table_length; ++i) {
        if (strcmp(str, inst_table[i]->str) == 0) 
            return i;
    }

    // 기계어 코드를 찾지 못한 경우
    return -1;
}

/**
 * @brief 소스코드 명령어 앞에 OPCODE가 기록된 코드를 파일에 출력한다.
 * `output_dir`이 NULL인 경우 결과를 stdout으로 출력한다. 프로젝트 1에서는
 * 불필요하다.
 *
 * @param output_dir 코드를 저장할 파일 경로, 혹은 NULL
 * @param tokens 토큰 테이블 주소
 * @param tokens_length 토큰 테이블의 길이
 * @param inst_table 기계어 목록 테이블 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 소스코드 명령어 앞에 OPCODE가 기록된 코드를 파일에 출력한다. `output_dir`이
 * NULL인 경우 결과를 stdout으로 출력한다. 명세서에 주어진 출력 예시와 완전히
 * 동일할 필요는 없다. 프로젝트 1에서는 불필요하다.
 */
int make_opcode_output(const char *output_dir, const token *tokens[],
                       int tokens_length, const inst *inst_table[],
                       int inst_table_length) {
    /* add your code */
}


/**
 * @brief 어셈블리 코드을 위한 패스 2 과정을 수행한다.
 *
 * @param tokens 토큰 테이블 주소
 * @param tokens_length 토큰 테이블 길이
 * @param inst_table 기계어 목록 테이블 주소
 * @param inst_table_length 기계어 목록 테이블 길이
 * @param symbol_table 심볼 테이블 주소
 * @param symbol_table_length 심볼 테이블 길이
 * @param literal_table 리터럴 테이블 주소
 * @param literal_table_length 리터럴 테이블 길이
 * @param obj_code 오브젝트 코드에 대한 정보를 저장하는 구조체 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행한다. 패스 2의
 * 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
 */
int assem_pass2(const token *tokens[], int tokens_length,
                const inst *inst_table[], int inst_table_length,
                const symbol *symbol_table[], int symbol_table_length,
                const literal *literal_table[], int literal_table_length,
                object_code *obj_code) {
    /* add your code */
    
    int start_address = 0;
    int next_locctr = 0;
    char object_code_buffer[MAX_OBJECT_CODE_STRING];
    int num_text_records = 0;
    obj_code->num_sections = 0;

    memset(obj_code, 0, sizeof(obj_code));

    for (int i = 0; i < tokens_length; i++) {
        if (tokens[i] == NULL) continue;

        int opcode = 0, format = 0, nixbpe = 0, disp = 0;
        char record_type = 'T'; // 기본은 텍스트 레코드

        // 1. LOCCTR 지정
        if (tokens[i]->operator != NULL) {
            if (tokens[i]->operator[0] == '+') {  // 4형식
                int index = search_opcode(tokens[i]->operator + 1, inst_table, inst_table_length);
                opcode = inst_table[index]->op;
                format = 4;
                next_locctr += format;
            } else {
                int index = search_opcode(tokens[i]->operator, inst_table, inst_table_length);
                if (index != -1) {
                    opcode = inst_table[index]->op;
                    format = inst_table[index]->format;
                    next_locctr += format;
                } else {
                    // 지시어 처리
                    if (strcmp(tokens[i]->operator, "START") == 0 || strcmp(tokens[i]->operator, "CSECT") == 0) {
                        next_locctr = 0;
                        obj_code->num_sections++;
                        record_type = 'S';
                        continue;
                    } else if (strcmp(tokens[i]->operator, "EXTDEF") == 0) {
                        // Define Record 생성
                        record_type = 'D';
                        continue;
                    } else if (strcmp(tokens[i]->operator, "EXTREF") == 0) {
                        // Reference Record 생성
                        record_type = 'R';
                        continue;
                    } else if (strcmp(tokens[i]->operator, "RESB") == 0) {
                        next_locctr += atoi(tokens[i]->operand[0]);
                        continue;
                    } else if (strcmp(tokens[i]->operator, "RESW") == 0) {
                        next_locctr += 3 * atoi(tokens[i]->operand[0]);
                        continue;
                    } else if (strcmp(tokens[i]->operator, "EQU") == 0) {
                        continue;  // EQU 처리는 pass1에서 주로 처리됨
                    }
                }
            }
            //printf("line %d: %04X\n", i + 1, next_locctr);

            // 2. nixbpe 및 disp 계산
            nixbpe = calculate_nixbpe(tokens[i]->operand[0], tokens[i]->operator, format);
            disp = calculate_target_address(tokens[i]->operand[0], next_locctr, format, symbol_table, symbol_table_length, literal_table, literal_table_length);
            
            //printf("line %d's target_address: %X\n", i + 1, disp);

            // 3. 오브젝트 코드 형식화
            format_object_code(object_code_buffer, tokens[i], opcode, nixbpe, disp, format);
            sprintf(obj_code->text_records[num_text_records++], "%06X^%02X^%s", next_locctr, format, object_code_buffer);
            
            printf("line %d: %s\n", i+1, object_code_buffer);

            int section_index = obj_code->num_sections;
            // 4. 적절한 레코드에 작성 
            add_record(obj_code, record_type, section_index, object_code_buffer);
        
            
        }
    }
    
    return 0;
    
}

void format_object_code(char *buffer, token *tok, int opcode, int nixbpe, int disp, int format) {
    if (format == 2) {
        // 2형식 명령어: opcode (8 bits), r1 (4 bits), {r2 (4 bits)}
        int r1, r2 = 0;
        if (tok->operand[0] != NULL) {
            if (strcmp(tok->operand[0], "A") == 0) { r1 = 0;}
            else if (strcmp(tok->operand[0], "X") == 0) { r1 = 1;}
            else if (strcmp(tok->operand[0], "S") == 0) { r1 = 4;}
            else if (strcmp(tok->operand[0], "T") == 0) { r1 = 5;}
        }
        if (tok->operand[1] != NULL) {
            if (strcmp(tok->operand[1], "A") == 0) { r2 = 0;}
            else if (strcmp(tok->operand[1], "X") == 0) { r2 = 1;}
            else if (strcmp(tok->operand[1], "S") == 0) { r2 = 4;}
            else if (strcmp(tok->operand[1], "T") == 0) { r2 = 5;}
        } else if (tok->operand[1] == NULL) {
            r2 = 0;
        }
        sprintf(buffer, "%02X%01X%01X", opcode, r1, r2);
    
    } else if (format == 3) {
        // 3형식 명령어: opcode (6 bits), nixbpe (6 bits), displacement (12 bits)
        sprintf(buffer, "%01X%01X%01X%03X", 
                opcode >> 4, // opcode의 상위 4비트
                (opcode & 0x0F) | ((nixbpe >> 4) & 0x0F), // opcode의 하위 4비트, nixbpe의 상위 2비트 결합
                (nixbpe & 0x0F), // nixbpe의 하위 4비트 
                disp & 0xFFF); // 주소 12비트 
    } else if (format == 4) {
        // 4형식 명령어: opcode (6 bits), nixbpe (6 bits), address (20 bits)
        sprintf(buffer, "%01X%01X%01X%05X", 
                opcode >> 4, // opcode의 상위 4비트
                (opcode & 0x0F) | ((nixbpe >> 4) & 0x0F), // opcode의 하위 4비트, nixbpe의 상위 2비트 결합
                (nixbpe & 0x0F), // nixbpe의 하위 4비트 
                disp & 0xFFFFF); // 주소 20비트 
    }
}

// 심볼 테이블에서 주어진 심볼에 대한 주소를 검색하는 함수
int search_symtab(const char *str, const symbol *symbol_table[], int symbol_table_length) {
    // 심볼 테이블을 순회하며 주어진 심볼을 검색
    for (int i = 0; i < symbol_table_length; i++) {
        if (strcmp(symbol_table[i]->name, str) == 0) {
            // 주어진 심볼을 찾은 경우 해당 심볼의 주소를 반환
            return symbol_table[i]->addr;
        }
    }
    // 주어진 심볼을 찾지 못한 경우 -1 반환 
    return -1;
}

// nixbpe 값을 계산하는 함수
int calculate_nixbpe(const char *operand, const char *operator, int format) {
    int nixbpe = 0;

    if (strcmp(operator, "RSUB") == 0) {
        nixbpe = 0x30; // n=1, i=1
        return nixbpe;
    } else if (operand == NULL) {
        return 0; 
    }

    // Immediate addressing
    if (operand[0] == '#') {
        nixbpe = 0x10; // n=0, i=1
    }
    // Indirect addressing
    else if (operand[0] == '@') {
        nixbpe = 0x20; // n=1, i=0
    }
    // Simple or direct addressing
    else {
        if (format != 4) {
            nixbpe = 0x32; // n=1, i=1, p=1
        } else {
            nixbpe = 0x31; // n=1, i=1, e=1
        }
    }

    // Indexed addressing 
    if (strstr(operand, "X") != NULL || strstr(operand, "x") != NULL) {
        nixbpe |= 0x08; // x=1
    }

    return nixbpe;
}

// 타겟 주소를 계산하는 함수
int calculate_target_address(const char *operand, int locctr, int format, const symbol *symbol_table[], int symbol_table_length, const literal *literal_table[], int literal_table_length) {
    if (operand == NULL) {
        return 0; // No operand, thus no target address
    }

    char clean_operand[256]; // To store operand without addressing symbols
    strcpy(clean_operand, operand);

    // Handle numeric operands directly in immediate addressing
    if (clean_operand[0] == '#') {
        // Skip the '#' and check if it's numeric
        if (isdigit(clean_operand[1])) {
            return atoi(clean_operand + 1); // Convert immediate number to integer
        } else {
            // It's a symbol after '#', search it in the symbol table
            clean_operand[0] = ' '; // Remove '#' for symbol table search
            return search_symtab(clean_operand + 1, symbol_table, symbol_table_length) - locctr;
        }
    }

    // 주소 지정 방식 기호 제거 
    if (clean_operand[0] == '@' || clean_operand[0] == '#') {
        memmove(clean_operand, clean_operand + 1, strlen(clean_operand)); // Shift left
    }

    // 심볼인 경우 
    int sym_address = search_symtab(clean_operand, symbol_table, symbol_table_length);
    if (sym_address != -1) {
        if (format != 4) {
            return sym_address - locctr;
        } else {
            return sym_address;
        }
        
    }

    // 리터럴인 경우
    int lit_address = search_littab(clean_operand, literal_table, literal_table_length);
    if (lit_address != -1) {
        if (format != 4) {
            return lit_address - locctr;
        } else {
            return lit_address;
        }
        
    }

    // If not found, it's an error or unresolved symbol
    return -1;
}

// 리터럴 테이블에서 주어진 리터럴에 대한 주소를 검색하는 함수
int search_littab(const char *str, const literal *literal_table[], int literal_table_length) {
    // 리터럴 테이블을 순회하며 주어진 리터럴을 검색
    for (int i = 0; i < literal_table_length; i++) {
        if (strcmp(literal_table[i]->literal, str) == 0) {
            // 주어진 리터럴을 찾은 경우 해당 리터럴의 주소를 반환
            return literal_table[i]->addr;
        }
    }
    // 주어진 리터럴을 찾지 못한 경우 에러 처리 (이 예시에서는 -1을 반환)
    return -1;
}

/**
 * 각 레코드 유형에 따라 오브젝트 코드 구조체에 데이터를 추가하는 함수.
 * 
 * @param obj_code 오브젝트 코드 구조체의 주소.
 * @param record_type 레코드 유형 (헤더, 텍스트, 엔드, 정의, 참조, 수정).
 * @param section_index 현재 처리중인 컨트롤 섹션의 인덱스.
 * @param data 레코드에 추가할 데이터.
 */
void add_record(object_code *obj_code, char record_type, int section_index, const char *data) {
    if (section_index < 0 || section_index >= MAX_CONTROL_SECTION_NUM) {
        fprintf(stderr, "Invalid section index: %d\n", section_index);
        return;
    }

    switch (record_type) {
        case HEADER_RECORD:
            strncpy(obj_code->header_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_header_records++;
            break;
        case DEFINE_RECORD:
            strncpy(obj_code->define_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_define_records++;
            break;
        case REFERENCE_RECORD:
            strncpy(obj_code->reference_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_reference_records++;
            break;
        case TEXT_RECORD:
            strncpy(obj_code->text_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_text_records++;
            break;
        case END_RECORD:
            strncpy(obj_code->end_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_end_records++;
            break;
        case MODIFY_RECORD:
            strncpy(obj_code->modify_records[section_index], data, MAX_OBJECT_CODE_STRING);
            obj_code->num_modify_records++;
            break;
        default:
            fprintf(stderr, "Unknown record type: %c\n", record_type);
            break;
    }
}

/**
 * @brief 심볼 테이블을 파일로 출력한다. `symbol_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다.
 *
 * @param symbol_table_dir 심볼 테이블을 저장할 파일 경로, 혹은 NULL
 * @param symbol_table 심볼 테이블 주소
 * @param symbol_table_length 심볼 테이블 길이
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 심볼 테이블을 파일로 출력한다. `symbol_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서에 주어진 출력 예시와 완전히 동일할 필요는 없다.
 */
int make_symbol_table_output(const char *symbol_table_dir,
                             const symbol *symbol_table[],
                             int symbol_table_length) {
    /* add your code */
    FILE* fp;

    if (symbol_table_dir == NULL) {
        fp = stdout; 
    } else {
        fp = fopen(symbol_table_dir, "w");
        if (fp == NULL) {
            perror("output_symtab.txt 읽기 실패\n");
            return -1;
        }
    }

    for (int i = 0; i < symbol_table_length; i++) {
        fprintf(fp, "%s\t%X\n", symbol_table[i]->name, symbol_table[i]->addr);
    }

    fclose(fp);

    return 0;
}

/**
 * @brief 리터럴 테이블을 파일로 출력한다. `literal_table_dir`이 NULL인 경우
 * 결과를 stdout으로 출력한다.
 *
 * @param literal_table_dir 리터럴 테이블을 저장할 파일 경로, 혹은 NULL
 * @param literal_table 리터럴 테이블 주소
 * @param literal_table_length 리터럴 테이블 길이
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 리터럴 테이블을 파일로 출력한다. `literal_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서에 주어진 출력 예시와 완전히 동일할 필요는 없다.
 */
int make_literal_table_output(const char *literal_table_dir,
                              const literal *literal_table[],
                              int literal_table_length) {
    /* add your code */
    FILE* fp;

    if (literal_table_dir == NULL) {
        fp = stdout; 
    } else {
        fp = fopen(literal_table_dir, "w");
        if (fp == NULL) {
            perror("output_littab.txt 읽기 실패\n");
            return -1;
        }
    }
    for (int i = 0; i < literal_table_length; i++) {
        fprintf(fp, "%s\t%X\n", literal_table[i]->literal, literal_table[i]->addr);
    }

    fclose(fp);

    return 0;
}

/**
 * @brief 오브젝트 코드를 파일로 출력한다. `objectcode_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다.
 *
 * @param objectcode_dir 오브젝트 코드를 저장할 파일 경로, 혹은 NULL
 * @param obj_code 오브젝트 코드에 대한 정보를 담고 있는 구조체 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 오브젝트 코드를 파일로 출력한다. `objectcode_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서의 주어진 출력 결과와 완전히 동일해야 한다.
 * 예외적으로 각 라인 뒤쪽의 공백 문자 혹은 개행 문자의 차이는 허용한다.
 */
int make_objectcode_output(const char *objectcode_dir,
                           const object_code *obj_code) {
    /* add your code */
    FILE* fp;

    if (objectcode_dir == NULL) {
        fp = stdout; 
    } else {
        fp = fopen(objectcode_dir, "w");
        if (fp == NULL) {
            return -1;
        }
    }

    int csect_num = obj_code->num_sections;

    for (int i = 0; i < csect_num; i++) {
        // 헤더 레코드 출력
        fprintf(fp, "%s\n", obj_code->header_records[i]);

        // 정의 레코드 출력
        if (obj_code->define_records[i][0] != '\0') { 
            for (int j = 0; j < obj_code->num_define_records; j++) {
                if (obj_code->define_records[i][j] != '\0')
                   fprintf(fp, "%s\n", obj_code->define_records[i][j]);
            }
        }

        // 참조 레코드 출력
        if (obj_code->reference_records[i][0] != '\0') { 
            for (int j = 0; j < obj_code->num_reference_records; j++) {
                fprintf(fp, "%s\n", obj_code->reference_records[i][j]);
            }
        }
        
        // 텍스트 레코드 출력
        for (int i = 0; i < obj_code->num_text_records; i++) {
            for (int j = 0; j < obj_code->num_text_records; j++) {
                fprintf(fp, "%s\n", obj_code->text_records[i][j]);
            }
        }

        // 수정 레코드 출력
        if (obj_code->modify_records[i][0] != '\0') { 
            for (int j = 0; j < obj_code->num_modify_records; j++) {
                fprintf(fp, "%s\n", obj_code->modify_records[i][j]);
            }
        }

        // 엔드 레코드 출력
        fprintf(fp, "%s\n", obj_code->end_records[i]);

    }


    fclose(fp);

    return 0;
}
