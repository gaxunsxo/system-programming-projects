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
#include "my_assembler_20221846.h"

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
// 추가로 선언한 함수
static int set_nixbpe(token *tok, const inst *inst_table[], int inst_table_length);
static int process_token(const token *tok, const inst *inst_table[], int inst_table_length, 
                        symbol *symbol_table[], int *num_symbols, 
                        literal *literal_table[], int *num_literals, int locctr, char *current_csect);

void add_symbol(symbol **sym_table, int *sym_count, const char *name, int addr, const char *csect);
void add_literal(literal **lit_table, int *lit_count, const char *lit, int addr);

symbol* search_symbol(symbol** symbol_table, int num_symbols, const char* name, const char* csect);
literal* search_literal(literal** literal_table, int num_literals, const char* name);
void generate_directive_object_code(char *buffer, const token *tok,
                                    literal *literal_table[], int literal_table_length);
int reg_code(const char *reg);
void format3or4(char *buffer, const token *tok, int opcode, int format, int locctr,
                const symbol *symbol_table[], int symbol_table_length, 
                const literal *literal_table[], int literal_table_length, const char *current_csect);

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
    int next_locctr = 0;
    int num_tokens = 0;
    int num_symbols = 0;
    int num_literals = 0;
    char current_csect[20] = "DEFAULT"; // 현재 컨트롤 섹션 이름, 초기값은 "DEFAULT"

    // 심볼 테이블, 리터럴 테이블 동적 할당
    *symbol_table = (symbol*)calloc(input_length, sizeof(symbol));
    *literal_table = (literal*)calloc(input_length, sizeof(literal));

    // 입력된 각 라인에 대해 토큰을 생성하고 PASS1 수행
    for (int i = 0; i < input_length; ++i) {
        line = strdup(input[i]); // 입력 라인 복사
        if (line == NULL) {
            fprintf(stderr, "라인 %d에서 메모리 할당 실패\n", i);
            return -1;  // 메모리 할당 실패 처리
        }

        // 개행 문자 제거
        size_t line_length = strlen(line);
        if (line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0'; // 개행 문자 제거
        }

        tokens[i] = (token *)malloc(sizeof(token));
        if (tokens[i] == NULL) {
            fprintf(stderr, "라인 %d에서 메모리 할당 실패. \n", i);
            return -1; 
        }

        init_token(tokens[i]);

        err = token_parsing(input[i], tokens[i], inst_table, inst_table_length);
        if (err != 0) {
            fprintf(stderr, "라인 %d에서 오류 %d로 파싱 실패. \n", i, err);
            return err;
        }
        num_tokens++;

        // 각 토큰 라인의 nixbpe 필드 초기화  
        tokens[i]->nixbpe = set_nixbpe(tokens[i], inst_table, inst_table_length);

        // 심볼 및 리터럴 처리 
        locctr = process_token(tokens[i], inst_table, inst_table_length,
                            symbol_table, &num_symbols, literal_table, &num_literals, locctr, current_csect);
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
int token_parsing(const char *input, token *tok, const inst *inst_table[], int inst_table_length) 
{
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
 * @brief 토큰 라인 별로 nixbpe 비트를 설정한다. 
*/
static int set_nixbpe(token *tok, const inst *inst_table[], int inst_table_length) {
    const char *operand = tok->operand[0];
    const char *operator = tok->operator;
    if (operator == NULL) {
        return 0;
    }
    int format;
    int index = search_opcode(operator, inst_table, inst_table_length);
    if (index != -1) {
        if (operator[0] == '+') {
            format = 4;
        } else {
            format = inst_table[index]->format;
        }
    }

    int nixbpe = 0;

    if (strcmp(operator, "RSUB") == 0) {
        nixbpe = 0x30; // n=1, i=1
    } else if (operand == NULL) {
        ;
    } else {
        // Immediate addressing 
        if (operand[0] == '#') {
            nixbpe = 0x10; // n=0, i=1
            if (isalpha(operand[1])) {
                // 문자일 경우 PC-relative 
                nixbpe |= 0x02;
            }
        }   
        // Indirect addressing
        else if (operand[0] == '@') {
            nixbpe = 0x20; // n=1, i=0
            if (isalpha(operand[1])) {
                // 문자일 경우 PC-relative 
                nixbpe |= 0x02;
            }
        }
        // Simple or Direct addrssing
        else {
            if (format == 4) {
                nixbpe = 0x31; // n=1, i=1, e=1
            } else {
                nixbpe = 0x32; // n=1, i=1, p=1
            }
        }

        // Indexed addressing
        const char* operand_2 = tok->operand[1];
        if (operand_2 != NULL && strcmp(operand_2, "X") == 0) {
            nixbpe |= 0x08; // x=1
        }
    }

    return nixbpe;
}

/**
 * @brief 토큰 라인을 읽어 심볼 테이블 및 리터럴 테이블을 생성한다. 
 * @return 다음 토큰 라인의 LOCCTR 
*/
static int process_token(const token *tok, const inst *inst_table[], int inst_table_length,
                        symbol **symbol_table, int *num_symbols, 
                        literal **literal_table, int *num_literals, int locctr, char *current_csect) {
    const char *label = tok->label;
    const char *operator = tok->operator;
    const char *operand = tok->operand[0];
    if (operator == NULL) { return locctr;}
    int next_locctr = locctr;

    // LOCCTR 계산
    if (operator != NULL) {
        if (tok->nixbpe & 0x01) {
            // 4형식 명령어인 경우
            locctr += 4;
        } else {
            // 2, 3형식 명령어인 경우
            int index = search_opcode(operator, inst_table, inst_table_length);
            if (index != -1) {
                locctr += inst_table[index]->format;
            } else {
                // 지시어 처리 
                if (strcmp(operator, "START") == 0) {
                    locctr = atoi(operand);
                    strcpy(current_csect, label); // 새로운 컨트롤 섹션 이름 설정
                } else if (strcmp(operator, "CSECT") == 0) {
                    locctr = 0;
                    next_locctr = 0;
                    strcpy(current_csect, label); // 새로운 컨트롤 섹션 이름 설정
                } else if (strcmp(operator, "EXTDEF") == 0 || strcmp(operator, "EXTREF") == 0) {
                    ;
                } else if (strcmp(operator, "RESB") == 0) {
                    locctr += atoi(operand);
                } else if (strcmp(operator, "RESW") == 0) {
                    locctr += 3 * atoi(operand);
                } else if (strcmp(operator, "BYTE") == 0) {
                    int operand_length = strlen(operand);
                    if (operand[0] == 'X') { 
                        // 두 글자당 한 바이트를 할당
                        locctr += (operand_length - 3) / 2;
                    } else if (operand[0] == 'C') {
                        // 한 글자당 한 바이트를 할당
                        locctr += operand_length - 3;
                    }
                } else if (strcmp(operator, "WORD") == 0) {
                    locctr += 3;
                } else if (strcmp(operator, "LTORG") == 0 || strcmp(operator, "END") == 0) {
                    // 리터럴 풀의 주소값을 계산하고 리터럴 테이블에 주소값을 할당
                    for (int k = 0; k < *num_literals; k++) {
                        // 주소가 할당되지 않은 리터럴이 있는 경우 
                        if (literal_table[k]->addr == -1) {
                            literal_table[k]->addr = locctr;
                        }
                            
                        // 리터럴 값에서 실제 데이터 부분 추출
                        const char *lit_value = literal_table[k]->literal;
                        char type = lit_value[1];  // 리터럴 타입: 'C' 또는 'X'
                        const char* content = lit_value + 3;  // 실제 데이터 시작 위치
                        int content_length = strlen(content) - 1;  // 마지막 따옴표 제거

                        if (type == 'X') {
                            // 16진수 리터럴, 두 글자당 1바이트
                            locctr += (content_length + 1) / 2;  // 홀수 길이 처리를 위해 반올림
                            break;
                        } else if (type == 'C') {
                            // 문자 리터럴, 각 글자당 1바이트
                            locctr += content_length;
                            break;
                        }
                    }
                } else if (strcmp(operator, "EQU") == 0) {
                    if (operand[0] == '*') {
                        // 피연산자가 '*'인 경우
                        ;
                    } else {
                        char *operand_copy = strdup(operand);
                        if (operand_copy == NULL) {
                            fprintf(stderr, "메모리 할당 실패\n");
                            return -1;
                        } 
                        
                        char *token = strtok(operand_copy, "+-");
                        int result = 0;
                        bool first = true;
                        char* next_ptr;

                        while (token) {
                            symbol* sym = search_symbol(symbol_table, *num_symbols, token, current_csect);
                            if (sym == NULL) {
                                fprintf(stderr, "심볼 %s을(를) 찾을 수 없습니다.\n", token);
                                free(operand_copy);
                                return -1;
                            }

                            // 연산 처리
                            int sym_value = sym->addr;
                            if (first) {
                                result = sym_value;
                                first = false;
                            } else {
                                char operation = operand[strlen(token)];  // strtok 이전에 토큰의 바로 앞 문자(연산자) 접근
                                if (operation == '-') {
                                    result -= sym_value;
                                } else if (operation == '+') {
                                    result += sym_value;
                                }
                            }

                            token = strtok(NULL, "+-");
                        }

                        free(operand_copy);
                        next_locctr = result;
                    }
                } 
            }
        }
    }

    // 라벨이 있는 경우 -> 심볼 테이블에 추가
    if (label != NULL && label[0] != '\0') { 
        add_symbol(symbol_table, num_symbols, label, next_locctr, current_csect);
    }

    // 피연산자가 리터럴인 경우 -> 리터럴 테이블에 추가
    if (operand != NULL && operand[0] == '=') {
        add_literal(literal_table, num_literals, operand, -1); // 리터럴의 주소 초기값: -1
    }

    return locctr;
}

/**
 * @brief 심볼 테이블을 생성한다.
*/
void add_symbol(symbol **symbol_table, int *num_symbols, const char *label, int locctr, const char *csect) {
    // 심볼 객체에 메모리 할당
    symbol_table[*num_symbols] = (symbol*)malloc(sizeof(symbol));
    if (symbol_table[*num_symbols] == NULL) {
        fprintf(stderr, "메모리 할당 실패.\n");
        return;
    }

    // 심볼 이름, 주소, 컨트롤 섹션 이름 할당
    strcpy(symbol_table[*num_symbols]->name, label);
    symbol_table[*num_symbols]->addr = locctr;
    strcpy(symbol_table[*num_symbols]->csect, csect);
    (*num_symbols)++;
}

/**
 * @brief 리터럴 테이블을 생성한다.
*/
void add_literal(literal **literal_table, int *num_literals, const char *operand, int locctr) {
    // 중복된 리터럴인지 확인 
    if (*num_literals) {
        for (int i = 0; i < *num_literals; i++) {
            const char *literal = literal_table[i]->literal;
            if (strcmp(literal, operand) == 0) {
                return;
            }
        }
    }
    
    // 리터럴 객체에 메모리 할당
    literal_table[*num_literals] = (literal*)malloc(sizeof(literal));
    if (literal_table[*num_literals] == NULL) {
        fprintf(stderr, "메모리 할당 실패.\n");
        return;
    }

    // 리터럴 이름과 주소 할당
    strcpy(literal_table[*num_literals]->literal, operand);
    literal_table[*num_literals]->addr = locctr;
    literal_table[*num_literals]->isProcessed = false;
    (*num_literals)++;
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
 * @brief 심볼 테이블에서 주어진 이름과 컨트롤 섹션의 심볼을 검색한다.
 * @return 찾은 심볼의 포인터, 찾지 못하면 NULL 반환
 */
symbol* search_symbol(symbol** symbol_table, int num_symbols, const char* name, const char* csect) {
    for (int i = 0; i < num_symbols; i++) {
        if (symbol_table[i] == NULL) continue;
        if (symbol_table[i]->name[0] == '\0' || symbol_table[i]->csect[0] == '\0') continue;
        if (strcmp(symbol_table[i]->name, name) == 0 && strcmp(symbol_table[i]->csect, csect) == 0) {
            return symbol_table[i];  // 찾은 심볼의 포인터 반환
        }
    }
    return NULL;  // 찾지 못하면 NULL 반환
}

/**
 * @brief 리터럴 테이블에서 주어진 이름의 리터럴을 검색한다.
 * @return 찾은 리터럴의 포인터, 찾지 못하면 NULL 반환
 */
literal* search_literal(literal** literal_table, int num_literals, const char* name) {
    for (int i = 0; i < num_literals; i++) {
        if (strcmp(literal_table[i]->literal, name) == 0) {
            return literal_table[i]; // 찾은 리터럴의 포인터 반환
        }
    }

    return NULL; // 찾지 못하면 NULL 반환 
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
    char buffer[100]; // 오브젝트 코드를 임시로 저장할 버퍼
    int start_address = 0; // 각 컨트롤 섹션의 시작 주소 
    int csect_length = 0; // 각 컨트롤 섹션의 총 길이 
    char current_csect[20] = "DEFAULT"; // 현재 컨트롤 섹션 이름, 초기값은 "DEFAULT"

    // 임시 수정 가능한 symbol_table과 literal_table 생성
    symbol **temp_symbol_table = malloc(symbol_table_length * sizeof(symbol*));
    literal **temp_literal_table = malloc(literal_table_length * sizeof(literal*));
    
    // 원본 데이터를 임시 테이블에 복사
    for (int i = 0; i < symbol_table_length; i++) {
        temp_symbol_table[i] = (symbol*) symbol_table[i];
    }
    for (int j = 0; j < literal_table_length; j++) {
        temp_literal_table[j] = (literal*) literal_table[j];
    }

    // 오브젝트 코드 구조체 초기화
    memset(obj_code, 0, sizeof(object_code));
    obj_code->num_sections = 1;

    char text_record[MAX_OBJECT_CODE_LENGTH * 10] = "";
    int text_record_length = 0;
    int text_record_start_address = 0;
    int current_locctr = 0;
    int current_section = 0;

    for (int i = 0; i < tokens_length; i++) {
        const token *tok = tokens[i];
        const char *operator = tok->operator;
        if (operator == NULL) { continue; }
        const char *operand = tok->operand[0];
        int opcode;
        int format;
        int locctr;
        buffer[0] = '\0'; // 임시 버퍼 초기화 
        
        current_locctr = locctr;
        char prev_csect[20];
        strcpy(prev_csect, current_csect);
        locctr = process_token(tok, inst_table, inst_table_length, 
                                temp_symbol_table, &symbol_table_length, 
                                temp_literal_table, &literal_table_length, locctr, current_csect);
        int nixbpe;

        // Header 레코드 작성
        if (strcmp(operator,"CSECT") == 0 || strcmp(operator, "END") == 0) {
            if (text_record_length > 0) {
                sprintf(obj_code->text[current_section][obj_code->text_count[current_section]], "T%06X%02X%s", text_record_start_address, text_record_length / 2, text_record);
                obj_code->text_count[current_section]++;
                text_record[0] = '\0';
                text_record_length = 0;
            }

            csect_length = (strcmp(operator, "CSECT") == 0) ? current_locctr : locctr;
            sprintf(obj_code->header[current_section][obj_code->header_count[current_section]], "H%-6s%06X%06X", prev_csect, start_address, csect_length);
            obj_code->header_count[current_section]++;

            if (strcmp(operator, "CSECT") == 0) {
                current_section++;
                obj_code->num_sections++;
                start_address = locctr;
                text_record_start_address = locctr;
            }
        }

        // End 레코드 작성
        if (strcmp(operator, "START") == 0 || strcmp(operator, "CSECT") == 0) {
            if (strcmp(operator, "START") == 0) {
                sprintf(obj_code->end[current_section][obj_code->end_count[current_section]], "E%06X", start_address);
                obj_code->end_count[current_section]++;
            } else {
                sprintf(obj_code->end[current_section][obj_code->end_count[current_section]], "E");
                obj_code->end_count[current_section]++;
            }
        }

        // Define 레코드 작성 
        if (strcmp(operator, "EXTDEF") == 0) {
            sprintf(obj_code->define[current_section][obj_code->define_count[current_section]], "D");
            for (int i = 0; i < MAX_OPERAND_PER_INST && tok->operand[i] != NULL; i++) {
                for (int j = 0; j < symbol_table_length; j++) {
                    if (strcmp(symbol_table[j]->name, tok->operand[i]) == 0) {
                        sprintf(obj_code->define[current_section][obj_code->define_count[current_section]] + strlen(obj_code->define[current_section][obj_code->define_count[current_section]]),
                             "%s%06X", tok->operand[i], symbol_table[j]->addr);
                        break;
                    }
                }
            }
            obj_code->define_count[current_section]++;
            continue;
        }

        // Reference 레코드 작성
        if (strcmp(operator, "EXTREF") == 0) {
            sprintf(obj_code->reference[current_section][obj_code->reference_count[current_section]], "R");
            for (int j = 0; j < MAX_OPERAND_PER_INST && tok->operand[j]; j++) {
                sprintf(obj_code->reference[current_section][obj_code->reference_count[current_section]] + strlen(obj_code->reference[current_section][obj_code->reference_count[current_section]]),
                        "%-6s", tok->operand[j]);
            }
            obj_code->reference_count[current_section]++;
            continue;
        }

        // Modification 레코드 생성
        if (operator[0] == '+') {
            sprintf(obj_code->modification[current_section][obj_code->modify_count[current_section]], "M%06X05+%s", current_locctr + 1, operand);
            obj_code->modify_count[current_section]++;
        } else if (strcmp(operator, "WORD") == 0) {
            char *operand_copy = strdup(operand);
            char *token = strtok(operand_copy, "+-");
            bool first = true;
            char* next_ptr;

            while (token) {
                if (first) {
                    first = false;
                    sprintf(obj_code->modification[current_section][obj_code->modify_count[current_section]], "M%06X06+%s", current_locctr, token);
                    obj_code->modify_count[current_section]++;
                } else {
                    char op = operand[strlen(token)];
                    if (op == '-') {
                        sprintf(obj_code->modification[current_section][obj_code->modify_count[current_section]], "M%06X06-%s", current_locctr, token);
                        obj_code->modify_count[current_section]++;
                    } else {
                        sprintf(obj_code->modification[current_section][obj_code->modify_count[current_section]], "M%06X06+%s", current_locctr, token);
                        obj_code->modify_count[current_section]++;
                    }
                }

                token = strtok(NULL, "+-");
            }
            
            free(operand_copy);
        }

        // 지시어인 경우
        if (strcmp(operator, "LTORG") == 0 || strcmp(operator, "END") ==0 ||
            strcmp(operator, "BYTE") == 0 || strcmp(operator, "WORD") == 0) {
            // 지시어의 오브젝트 코드를 생성 
            generate_directive_object_code(buffer, tok, literal_table, literal_table_length);

            // Text 레코드 추가
            if (text_record_length + strlen(buffer) > 0x1E * 2) {
                sprintf(obj_code->text[current_section][obj_code->text_count[current_section]], "T%06X%02X%s", text_record_start_address, text_record_length / 2, text_record);
                obj_code->text_count[current_section]++;
                text_record[0] = '\0';
                text_record_length = 0;
                text_record_start_address = current_locctr;
            } else {
                strcat(text_record, buffer);
                text_record_length += strlen(buffer);
            }
            continue;
        }

        // 명령어인 경우: 정보 검색 
        int index = search_opcode(operator, inst_table, inst_table_length);
        if (index != -1) {
            format = operator[0] == '+' ? 4 : inst_table[index]->format;
            opcode = inst_table[index]->op;
        } else {
            continue;
        }
        
        // 명령어 형식에 따라 오브젝트 코드 생성
        switch (format) {
            case 1:
                sprintf(buffer, "%02X", opcode);
                break;
            case 2: 
                // 레지스터 코드 처리 필요 
                sprintf(buffer, "%02X%1X%1X", opcode, reg_code(tok->operand[0]), reg_code(tok->operand[1]));
                break;
            case 3: 
            case 4: 
                // TO-DO: 3, 4형식 명령어의 오브젝트 코드를 생성하는 함수 구현하기 
                nixbpe = tok->nixbpe;
                
                format3or4(buffer, tok, opcode, format, locctr,
                         symbol_table, symbol_table_length, literal_table, literal_table_length, current_csect);

                break;
            default: 
                fprintf(stderr, "지원하지 않는 명령어 형식입니다. %d\n", format);
                return -1;
        }

        // Text 레코드 추가
        if (text_record_length + strlen(buffer) > 0x1E * 2 || buffer[0] == '\0') {
            sprintf(obj_code->text[current_section][obj_code->text_count[current_section]], "T%06X%02X%s", text_record_start_address, text_record_length / 2, text_record);
            obj_code->text_count[current_section]++;
            text_record[0] = '\0';
            text_record_length = 0;
            text_record_start_address = current_locctr;
        }

        if (buffer[0] != '\0') {
            strcat(text_record, buffer);
            text_record_length += strlen(buffer);
        }
    }

    // 마지막 텍스트 레코드 추가 (있는 경우)
    if (text_record_length > 0) {
        sprintf(obj_code->text[current_section][obj_code->text_count[current_section]], "T%06X%02X%s", text_record_start_address, text_record_length / 2, text_record);
        obj_code->text_count[current_section]++;
    }

    return 0;
    
}

// 특정 지시어의 오브젝트 코드를 생성하는 함수
void generate_directive_object_code(char *buffer, const token *tok,
                                    literal *literal_table[], int literal_table_length) {
    const char* operator = tok->operator;
    const char* operand = tok->operand[0];

    // 리터럴 관련 처리
    if (strcmp(operator, "LTORG") == 0 || strcmp(operator, "END") == 0) {
        // 리터럴 테이블에 있는 모든 리터럴 처리
        for (int i = 0; i < literal_table_length; i++) {
            if (literal_table[i]->isProcessed) {
                continue;
            }

            // 리터럴 타입 확인
            const char* lit_value = literal_table[i]->literal;
            char type = lit_value[1];

            // 리터럴 값에서 실제 데이터 부분 추출
            const char* content = lit_value + 3; 
            size_t len = strlen(content) - 1;
            if (type == 'C') {
                // 문자 리터럴 처리 
                for (size_t j = 0; j < len; j++) {
                    char ch = content[j];
                    char hex[3];
                    snprintf(hex, sizeof(hex), "%02X", (unsigned char)ch);
                    strncat(buffer, hex, sizeof(hex));
                }
            } else if (type == 'X') {
                // 16진수 리터럴 처리
                strncat(buffer, content, len);
            }   
            
            literal_table[i]->isProcessed = true;
            break;     
        }
    }

    // BYTE, WORD 처리
    else if (strcmp(operator, "BYTE") == 0) {
        // BYTE 처리
        char type = operand[0];
        if (type == 'C') {
            // 문자 상수: ASCII 값으로 변환
            const char* characters = operand + 2;
            size_t len = strlen(characters) - 1;
            for (size_t j = 0; j < len; j++) {
                char ch = characters[j];
                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", (unsigned char)ch);
                strcat(buffer, hex);
            }
        } else if (type == 'X') {
            // 16진수 상수: 직접 사용
            const char* hex_data = operand + 2;
            size_t len = strlen(hex_data) - 1;
            strncat(buffer, hex_data, len);
        }
    } else if (strcmp(operator, "WORD") == 0) {
        // WORD 처리
        if (!isdigit(operand[0])) {
            operand = 0; // 기본값 = 0
        }
        sprintf(buffer, "%06X", atoi(operand));
    }
}

// 레지스터 코드를 반환하는 함수 
int reg_code(const char *reg) {
    if (reg == NULL) {
        return 0;
    }

    if (strcmp(reg, "A") == 0) return 0;
    if (strcmp(reg, "X") == 0) return 1;
    if (strcmp(reg, "L") == 0) return 2;
    if (strcmp(reg, "B") == 0) return 3;
    if (strcmp(reg, "S") == 0) return 4;
    if (strcmp(reg, "T") == 0) return 5;
    if (strcmp(reg, "F") == 0) return 6;

    else {
        return -1;
    }
}

// 3, 4형식 명령어의 오브젝트 코드를 생성하는 함수 
void format3or4 (char *buffer, const token *tok, int opcode, int format, int locctr,
                const symbol *symbol_table[], int symbol_table_length, 
                const literal *literal_table[], int literal_table_length, const char *current_csect) {
    int nixbpe = tok->nixbpe;
    const char* operand = tok->operand[0];
    const char* operator = tok->operator;
    bool isFormat4 = false;
    if (format == 4) 
        isFormat4 = true; 
    
    // 상위 3자리 코드 문자열 생성
    int opcode_high = opcode >> 4;
    int combine_bits = (opcode & 0x0F) | ((nixbpe >> 4) & 0x0F);
    int nixbpe_low = nixbpe & 0x0F;

    sprintf(buffer, "%1X%1X%1X", opcode_high, combine_bits, nixbpe_low);

    int address = 0;

    // 피연산자 유형 확인 및 주소 가져오기
    if (strcmp(operator, "RSUB") == 0) {
        address = 0;
        sprintf(buffer + 3, "%03X", address);
        return;
    } else if (isalpha(operand[0]) || operand[0] == '@') {
        if (operand[0] == '@') {
            operand = operand + 1;
        }
        // 심볼의 주소 찾기
        symbol* sym = search_symbol(symbol_table, symbol_table_length, operand, current_csect);
        if (sym != NULL) {
            address = sym->addr;
        } 
    } else if (operand[0] == '=') {
        // 리터럴의 주소 찾기 
        literal* lit = search_literal(literal_table, literal_table_length, operand);
        if (lit->addr != -1) {
            address = lit->addr;
        }
    } 

    // 주소 계산 로직 
    if (nixbpe & 0x01) {
        // 4형식 명령어
        address = 0;
    } else if (!(nixbpe & 0x20) && (nixbpe & 0x10)) {
        // 직접 주소 방식: n=0, i=1인 경우 
        address = atoi(operand + 1);
    } else if (nixbpe & 0x02 || nixbpe & 0x20) {
        // PC 상대 주소 계산
        address = (address - locctr) & 0xFFF;
    }  

    sprintf(buffer + 3, isFormat4 ? "%05X" : "%03X", address);
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
        fprintf(fp, "%s\t%X\t%s\n", symbol_table[i]->name, symbol_table[i]->addr, symbol_table[i]->csect);
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

    for (int section = 0; section < obj_code->num_sections; section++) {
        // Header 레코드 출력
        for (int i = 0; i < obj_code->header_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->header[section][i]);
        }

        // Define 레코드 출력
        for (int i = 0; i < obj_code->define_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->define[section][i]);
        }

        // Reference 레코드 출력
        for (int i = 0; i < obj_code->reference_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->reference[section][i]);
        }

        // Text 레코드 출력
        for (int i = 0; i < obj_code->text_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->text[section][i]);
        }

        // Modification 레코드 출력
        for (int i = 0; i < obj_code->modify_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->modification[section][i]);
        }

        // End 레코드 출력
        for (int i = 0; i < obj_code->end_count[section]; i++) {
            fprintf(fp, "%s\n", obj_code->end[section][i]);
        }
    }

    if (objectcode_dir != NULL) {
        fclose(fp);
    }

    return 0;
}