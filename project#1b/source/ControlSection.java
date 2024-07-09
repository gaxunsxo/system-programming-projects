import java.util.*;

public class ControlSection {
	/**
	 * pass1 작업을 수행한다. 기계어 목록 테이블을 통해 소스 코드를 토큰화하고, 심볼 테이블 및 리터럴 테이블을 초기화환다.
	 *
	 * @param instTable 기계어 목록 테이블
	 * @param input     하나의 control section에 속하는 소스 코드. 마지막 줄은 END directive를 강제로
	 *                  추가하였음.
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public ControlSection(InstructionTable instTable, ArrayList<String> input) throws RuntimeException {
		_instTable = instTable;
		_tokens = new ArrayList<Token>();
		_symbolTable = new SymbolTable();
		_literalTable = new LiteralTable();

		// TODO: pass1 수행하기.

		// 소스 코드 토큰화 및 테이블 초기화
		for (String line : input) {
			try {
				Token.setInstructionTable(_instTable); // Token 클래스에 기계어 목록 테이블 설정
				Token token = new Token(line, locctr);
				_tokens.add(token);

				// 심볼 테이블 및 리터럴 테이블 생성
				processToken(token);
			} catch (Exception e) {
				throw new RuntimeException("Error tokenizing source code: " + e.getMessage());
			}
		}
	}

	// 토큰을 처리해 심볼 테이블과 리터럴 테이블을 생성한다.
	private void processToken(Token token) {
		// 심볼 처리
		if (!token.getLabel().isEmpty()) {
			if (!token.getOperator().equals("EQU")) {
				// 일반 라벨을 심볼 테이블에 추가
				_symbolTable.putLabel(token.getLabel(), token.getLocctr());
			} else {
				// EQU 라벨 처리, EQU 뒤에 오는 값이나 표현식을 처리
				_symbolTable.putLabel(token.getLabel(), token.getLocctr(), token.getOperands().get(0));
			}
		}
		if (!token.getOperator().isEmpty() && token.getOperator().equals("EXTREF")) {
			// EXTREF에 명시된 모든 외부 참조 심볼을 추가
			for (String extRef : token.getOperands()) {
				_symbolTable.putRefer(extRef);
			}
		}
		// 리터럴 처리
		if (!token.getOperands().isEmpty() && token.getOperands().get(0).startsWith("=")) {
			// 리터럴을 리터럴 테이블에 추가 (주소는 아직 계산하지 않음)
			_literalTable.putLiteral(token.getOperands().get(0));
		}
		if (token.getOperator().equals("LTORG") || token.getOperator().equals("END")) {
			// LTORG 혹은 END 라벨을 만났을 때 리터럴 주소 계산
			_literalTable.assignAddresses(token.getLocctr());
		}
	}

	/**
	 * pass2 작업을 수행한다. pass1에서 초기화한 토큰 테이블, 심볼 테이블 및 리터럴 테이블을 통해 오브젝트 코드를 생성한다.
	 *
	 * @return 해당 control section에 해당하는 오브젝트 코드 객체
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public ObjectCode buildObjectCode() throws RuntimeException {
		ObjectCode objCode = new ObjectCode();

		// Header 레코드 생성
		objCode.appendHeader(_tokens.get(0).getLabel(),
				_tokens.get(0).getLocctr(), calculateControlSectionLength());

		// Text 레코드 생성
		StringBuilder textRecord = new StringBuilder();
		int recordLength = 0;
		int startAddress = _tokens.get(0).getLocctr();

		// TODO: pass2 수행하기.
		for (Token token : _tokens) {
			// Object Code 생성
			String objectCode = generateObjectCode(token);
			if (!objectCode.isEmpty()) {
				if (recordLength + objectCode.length() / 2 > 30) {
					// 최대 길이 초과 시 새 Text 레코드 시작
					objCode.appendTextRecord(startAddress, textRecord.toString(), recordLength);
					textRecord = new StringBuilder(objectCode);
					startAddress = token.getLocctr();
					recordLength = objectCode.length() / 2;
				} else {
					textRecord.append(objectCode);
					recordLength += objectCode.length() / 2;
				}
			} else {
				// 빈 오브젝트 코드를 만날 경우 다음 유효한 토큰의 위치를 새 시작 주소로 설정
				if (textRecord.length() > 0) {
					objCode.appendTextRecord(startAddress, textRecord.toString(), recordLength);
					textRecord = new StringBuilder();
					recordLength = 0;
				}
				startAddress = token.getNextLocctr(); // 다음 토큰의 위치를 새로운 시작 주소로 설정
			}

			// Define 레코드 생성
			if (token.getOperator().equals("EXTDEF")) {
				Map<String, Integer> symbols = new LinkedHashMap<>();
				for (String symbol : token.getOperands()) {
					symbols.put(symbol, _symbolTable.getAddress(symbol).orElseThrow(
							() -> new RuntimeException("A symbol that is not defined: " + symbol)
					));
				}
				objCode.appendDefineRecord(symbols);
			}

			// Reference 레코드 생성
			if (token.getOperator().equals("EXTREF")) {
				List<String> symbols = new ArrayList<>(token.getOperands());
				objCode.appendReferenceRecord(symbols);
			}
		}

		// 마지막 Text 레코드 추가 (있는 경우)
		if (recordLength > 0) {
			objCode.appendTextRecord(startAddress, textRecord.toString(), recordLength);
		}

		// Modification 레코드 생성
		objCode.appendModificationRecords(_tokens, _symbolTable);

		// End 레코드 생성
		objCode.appendEndRecord(_tokens);

		return objCode;
	}

	// 각 컨트롤 섹션의 전체 길이를 구하는 메소드
	public int calculateControlSectionLength() {
		// 컨트롤 섹션의 시작 주소
		int startAddress = _tokens.get(0).getLocctr();

		// "END" 명령어를 가진 토큰을 찾음
		Token endToken = _tokens.stream()
				.filter(token -> "END".equals(token.getOperator()))
				.findFirst()
				.orElseThrow(() -> new RuntimeException("END command not found."));

		int endAddress = endToken.getLocctr();

		// 컨트롤 섹션의 전체 길이를 초기 계산
		int totalLength = endAddress - startAddress;

		// 리터럴 테이블 탐색 및 조건에 맞는 리터럴의 길이를 계산하여 추가
		for (Literal literal : _literalTable.getAllLiterals()) {
			// 리터럴의 주소가 END 명령어의 locctr과 같은지 확인
			if (literal.getAddress().get().equals(endAddress)) {
				// 리터럴의 타입에 따라 길이를 계산
				String value = literal.getLiteral();
				if (value.startsWith("=C'") && value.endsWith("'")) {
					// 문자 리터럴인 경우
					totalLength += value.length() - 4;
				} else if (value.startsWith("=X'") && value.endsWith("'")) {
					// 16진수 리터럴인 경우
					totalLength += (value.length() - 4) / 2;
				}
			}
		}

		return totalLength;
	}

	/**
	 * 각 소스 코드 라인을 오브젝트 코드로 변환한다.
	 *
	 */
	public String generateObjectCode(Token token) {
		// 각 토큰 라인의 명령어, nixbpe, 피연산자를 가져온다
		// 해당 명령어의 형식을 가져와 각 형식에 따라 object code를 생성한다
		// nixbpe에 따라 address 값을 결정한다
		/**
		 * - 2형식인 경우: opcode (8bits) + r1 (4bits) + r2 (4bits)
		 * - 3형식인 경우: opcode (6bits) + nixbpe (6bits) + address (12bits)
		 * - 4형식인 경우: opocde (6bits) + nixbpe (6bits) + address (20bits)
		 */
		StringBuilder objectCode = new StringBuilder();
		String operator = token.getOperator();
		if (operator.isEmpty()) {
			return "";
		}
		List<String> operands = token.getOperands();
		int nixbpe = token.getNixbpe();

		// 지시어인 경우
		if (Token.directives.contains(operator)) {
			if(operator.equals("LTORG") || operator.equals("END")
			|| operator.equals("BYTE") || operator.equals("WORD")) {
				objectCode.append(generateDirectiveObjectCode(token));
				return objectCode.toString();
			} else {
				return ""; // 그 외의 지시어에는 빈 문자열 반환
			}
		}

		// 명령어 정보 검색
		Optional<InstructionInfo> instructionOpt =_instTable.search(operator);
		if (instructionOpt.isEmpty()) {
			throw new RuntimeException("Command information not found: " + operator);
		}

		InstructionInfo info = instructionOpt.get();
		int opcode = info.getOpcode();
		int format = token.isE() ? 4 : info.getFormat();

		switch (format) {
			case 2:
				objectCode.append(String.format("%02X", opcode));
				objectCode.append(format2(operands));
				break;
			case 3:
				objectCode.append(format3or4(token, opcode, false, nixbpe, operands));
				break;
			case 4:
				objectCode.append(format3or4(token, opcode, true, nixbpe, operands));
				break;
			default:
				throw new RuntimeException("Unsupported command format.");
		}
		return objectCode.toString();
	}

	// 특정 지시어의 object code 생성
	private String generateDirectiveObjectCode(Token token) {
		StringBuilder code = new StringBuilder();
		String operator = token.getOperator();
		List<String> operands = token.getOperands();

		// 리터럴 관련 처리
		if (operator.equals("LTORG")) {
			// 리터럴 테이블에서 모든 리터럴을 처리
			for (Literal literal : _literalTable.getAllLiterals()) {
				// 리터럴 오브젝트 코드 생성
				processLiteral(literal, code);
			}
			_literalTable.markedAsProcessed(); // 리터럴 처리 완료로 표시
		} else if (operator.equals("END")) {
			// 리터럴이 처리되지 않은 경우
			if (!_literalTable.isProcessed()) {
				for (Literal literal : _literalTable.getAllLiterals()) {
					processLiteral(literal, code);
				}
			}
		}
		// BYTE, WORD 처리
		else if (operator.equals("BYTE")) {
			String operand = operands.get(0);
			// BYTE 지시어의 경우 문자열 또는 16진수 데이터 처리
			if (operand.startsWith("C'") && operand.endsWith("'")) {
				// 문자 상수: ASCII 값으로 변환
				String characters = operand.substring(2, operand.length() - 1);
				for (char ch : characters.toCharArray()) {
					code.append(String.format("%02X", (int) ch));
				}
			} else if (operand.startsWith("X'") && operand.endsWith("'")) {
				// 16진수 상수: 직접 사용
				code.append(operand.substring(2, operand.length() - 1));
			}
		} else if (operator.equals("WORD")) {
			String operand = operands.get(0);
			int value = 0; // 숫자 형식이 아닌 경우 기본값으로 0 사용
			// WORD 지시어의 경우, 숫자를 3바이트 16진수 값으로 변환
			try {
				value = Integer.parseInt(operand); // 입력된 값이 숫자인 경우, 해당 값으로 설정
			} catch (NumberFormatException e) {
				// 예외가 발생해도 특별히 처리할 것이 없음, value는 이미 0으로 초기화됨
			}
			code.append(String.format("%06X", value));
		} else {
			// 그 외의 지시어인 경우 빈 문자열 반환
			return "";
		}
		return code.toString().trim();
	}

	// 리터럴 오브젝트 코드 처리 메소드
	private void processLiteral(Literal literal, StringBuilder code) {
		String literalValue = literal.getLiteral();
		if (literalValue.startsWith("=C'") && literalValue.endsWith("'")) {
			// 문자 리터럴 처리
			String characters = literalValue.substring(3, literalValue.length() - 1);
			for (char ch : characters.toCharArray()) {
				code.append(String.format("%02X", (int) ch));
			}
		} else if (literalValue.startsWith("=X'") && literalValue.endsWith("'")) {
			// 16진수 리터럴 처리
			code.append(literalValue.substring(3, literalValue.length() - 1));
		}
		code.append("\n"); // 각 리터럴을 분리
	}

	// 2형식 명령어의 object code 생성
	private String format2(List<String> operands) {
		if (operands.size() < 1 || operands.size() > 2) {
			throw new IllegalArgumentException("A two-form command must have no more than two operands.");
		}

		// 레지스터 테이블 초기화
		Map<String, Integer> registerTable = new HashMap<>();
		registerTable.put("A", 0);
		registerTable.put("X", 1);
		registerTable.put("L", 2);
		registerTable.put("B", 3);
		registerTable.put("S", 4);
		registerTable.put("T", 5);
		registerTable.put("F", 6);

		// 각 피연산자에 대한 레지스터 정보 확인
		StringBuilder code = new StringBuilder();
		for (String operand : operands) {
			Integer regCode = registerTable.get(operand);
			if (regCode == null) {
				throw new RuntimeException("Unsupported registers: " + operand);
			}
			code.append(String.format("%1X", regCode));
		}

		// 하나의 레지스터만 사용된 경우
		if (operands.size() == 1) {
			code.append("0"); // r2 = 0으로 처리
		}

		return code.toString();
	}

	// 3, 4형식 명령어의 object code 생성
	private String format3or4(Token token, int opcode, boolean isFormat4, int nixbpe, List<String> operands) {
		StringBuilder code = new StringBuilder();

		// 1. opcode의 상위 4비트(명령어 코드 부분) 추출
		int opcodeHigh = opcode >> 4;
		// 2. opcode의 상위 6비트 중 하위 2비트와 nixbpe의 하위 6비트 중 상위 2비트를 결합
		int combineBits = (opcode & 0x0F) | ((nixbpe >> 4) & 0x0F);
		// 3. nixbpe의 하위 4비트 추출
		int nixbpeLow = nixbpe & 0x0F;

		// 추출한 비트들을 사용하여 코드 문자열 생성
		code.append(String.format("%1X%1X%1X", opcodeHigh, combineBits, nixbpeLow));

		int address = 0;
		String operand = operands.get(0);
		Optional<Integer> operandAddress;

		// 피연산자 유형 확인 및 주소 가져오기
		if (Character.isLetter(operand.charAt(0))) {
			// 심볼의 주소 찾기
			operandAddress = _symbolTable.getAddress(operand);
			if (!operandAddress.isPresent()) {
				if (token.getOperator().equals("RSUB"))
					operandAddress = Optional.of(0);
				else
					throw new RuntimeException("\nAddress not found in symbol table: " + operand);
			}
		} else if (operand.startsWith("=")) {
			// 리터럴의 주소 찾기
			operandAddress = _literalTable.getAddress(operand);
			if (!operandAddress.isPresent()) {
				throw new RuntimeException("\nAddress not found in literal table: " + operand);
			}
		} else {
			// 숫자로 간주하여 즉시 값을 사용
			operandAddress = Optional.of(Integer.parseInt(operand));
		}

		// 주소 계산 로직
		int targetAddress = operandAddress.get();
		int next_locctr = token.getNextLocctr(); // 현재 라인의 다음 locctr을 가져옴
		if (token.isP() || token.isN()) {
			// PC 상대 주소 계산
			address = (targetAddress - next_locctr) & 0xFFF;
		} else {
			// 직접 주소 사용
			if (targetAddress == -1) // 피연산자가 외부 참조 심볼인 경우
				address = 0;
			else // 심볼의 주소
				address = targetAddress;
		}

		code.append(String.format(isFormat4 ? "%05X" : "%03X", address));
		return code.toString();
	}

	/**
	 * 심볼 테이블을 String으로 변환하여 반환한다. Assembler.java에서 심볼 테이블을 출력하는 데에 사용된다.
	 *
	 * @return 문자열로 변경된 심볼 테이블
	 */
	public String getSymbolString() {
		return _symbolTable.toString();
	}

	/**
	 * 리터럴 테이블을 String으로 변환하여 반환한다. Assembler.java에서 리터럴 테이블을 출력하는 데에 사용된다.
	 *
	 * @return 문자열로 변경된 리터럴 테이블
	 */
	public String getLiteralString() {
		return _literalTable.toString();
	}

	/** 기계어 목록 테이블 */
	private InstructionTable _instTable;

	/** 토큰 테이블 */
	private ArrayList<Token> _tokens;

	/** 심볼 테이블 */
	private SymbolTable _symbolTable;

	/** 리터럴 테이블 */
	private LiteralTable _literalTable;

	// LOCCTR 초기화
	private int[] locctr = new int[1];
}