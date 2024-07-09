/**
 * @file my_assembler_00000000.h
 * @date 2024-04-09
 * @version 0.1.0
 *
 * @brief my_assembler_00000000.c를 위한 매크로 및 구조체 선언부
 */

#ifndef __MY_ASSEMBLER_H__
#define __MY_ASSEMBLER_H__

#define MAX_INST_TABLE_LENGTH 256
#define MAX_INPUT_LINES 5000
#define MAX_TABLE_LENGTH 5000
#define MAX_OPERAND_PER_INST 3
#define MAX_OBJECT_CODE_STRING 74
#define MAX_OBJECT_CODE_LENGTH 5000
#define MAX_CONTROL_SECTION_NUM 10

/**
 * @brief 한 개의 SIC/XE instruction을 저장하는 구조체
 *
 * @details
 * 기계어 목록 파일(inst_table.txt)에 명시된 SIC/XE instruction 하나를
 * 저장하는 구조체. 라인별로 하나의 instruction을 저장하고 있는 instruction 목록
 * 파일로부터 정보를 받아와서 생성한다.
 */
typedef struct _inst {
    char str[10];     /** instruction 이름 */
    unsigned char op; /** instruction의 opcode */
    int format;       /** instruction의 format */
    int ops;          /** instruction이 가지는 operator 개수 */
} inst;

/**
 * @brief 소스코드 한 줄을 분해하여 저장하는 구조체
 *
 * @details
 * 원할한 assem을 위해 소스코드 한 줄을 label, operator, operand, comment로
 * 파싱한 후 이를 저장하는 구조체. 필드의 `operator`는 renaming을 허용한다.
 */
typedef struct _token {
    char *label;   /** label을 가리키는 포인터 */
    char *operator; /** operator를 가리키는 포인터 */
    char *operand[MAX_OPERAND_PER_INST]; /** operand들을
                                            가리키는 포인터 배열 */
    char *comment; /** comment를 가리키는 포인터 */
    char nixbpe;   /** 특수 bit 정보 */
} token;

/**
 * @brief 하나의 심볼에 대한 정보를 저장하는 구조체
 *
 * @details
 * SIC/XE 소스코드에서 얻은 심볼을 저장하는 구조체이다. 기존에 정의된 `name` 및
 * `addr`는 필수로 사용해야 한다. 필드가 더 필요한 경우 구조체 내에 필드를
 * 추가하는 것을 허용한다.
 */
typedef struct _symbol {
    char name[10]; /** 심볼의 이름 */
    int addr;      /** 심볼의 주소 */
    /* add fields if needed */
} symbol;

/**
 * @brief 하나의 리터럴에 대한 정보를 저장하는 구조체
 *
 * @details
 * SIC/XE 소스코드에서 얻은 리터럴을 저장하는 구조체이다. 기존에 정의된 literal
 * 및 addr는 필수로 사용하고, field가 더 필요한 경우 구조체 내에 field를
 * 추가하는 것을 허용한다. addr 필드는 리터럴의 값을 저장하는 것이 아닌 리터럴의
 * 주소를 저장하는 필드임을 유의하라.
 */
typedef struct _literal {
    char literal[20]; /** 리터럴의 표현식 */
    int addr;         /** 리터럴의 주소 */
    /* add fields if needed */
} literal;

/**
 * @brief 오브젝트 코드 전체에 대한 정보를 담는 구조체
 *
 * @details
 * 오브젝트 코드 전체에 대한 정보를 담는 구조체이다. Header Record, Define
 * Record, Modification Record 등에 대한 정보를 모두 포함하고 있어야 한다. 이
 * 구조체 변수 하나만으로 object code를 충분히 작성할 수 있도록 구조체를 직접
 * 정의해야 한다.
 */
typedef struct _object_code {
    /* add fields */
    char header_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1]; // 헤더 레코드 배열
    char define_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1]; // define 레코드 배열
    char reference_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1]; // reference 레코드 배열 
    char text_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1];   // 텍스트 레코드 배열
    char end_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1];    // 엔드 레코드 배열
    char modify_records[MAX_CONTROL_SECTION_NUM][MAX_OBJECT_CODE_STRING + 1]; // 모디파이 레코드 배열

    int num_header_records; // 헤더 레코드의 개수
    int num_define_records; // define 레코드의 개수
    int num_reference_records; // reference 레코드의 개수 
    int num_text_records;   // 텍스트 레코드의 개수
    int num_end_records;    // 엔드 레코드의 개수
    int num_modify_records; // 모디파이 레코드의 개수

    int num_sections; // Control Section의 개수
} object_code;

int init_inst_table(inst *inst_table[], int *inst_table_length,
                    const char *inst_table_dir);
int init_input(char *input[], int *input_length, const char *input_dir);
int assem_pass1(const inst *inst_table[], int inst_table_length,
                const char *input[], int input_length, token *tokens[],
                int *tokens_length, symbol *symbol_table[],
                int *symbol_table_length, literal *literal_table[],
                int *literal_table_length);
int token_parsing(const char *input, token *tok, const inst *inst_table[],
                  int inst_table_length);
int search_opcode(const char *str, const inst *inst_table[],
                  int inst_table_length);
int make_opcode_output(const char *output_dir, const token *tokens[],
                       int tokens_length, const inst *inst_table[],
                       int inst_table_length);
int assem_pass2(const token *tokens[], int tokens_length,
                const inst *inst_table[], int inst_table_length,
                const symbol *symbol_table[], int symbol_table_length,
                const literal *literal_table[], int literal_table_length,
                object_code *obj_code);
int make_symbol_table_output(const char *symbol_table_dir,
                             const symbol *symbol_table[],
                             int symbol_table_length);
int make_literal_table_output(const char *literal_table_dir,
                              const literal *literal_table[],
                              int literal_table_length);
int make_objectcode_output(const char *objectcode_dir,
                           const object_code *obj_code);

#endif
