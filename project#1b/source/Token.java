import java.util.*;
import java.util.stream.Collectors;

public class Token {
	/**
	 * 소스 코드 한 줄에 해당하는 토큰을 초기화한다.
	 * 
	 * @param input, int[] locctr 소스 코드 한 줄에 해당하는 문자열, 해당 라인의 locctr
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public Token(String input, int[] locctr) throws RuntimeException {
		// TODO: Token 클래스의 field 초기화.

		// 4형식 명령어 관리
		boolean isFormat4 = false;

		// locctr
		this.locctr = locctr;
		this.currentLocctr = locctr[0];

		// label - operator - operands - comment
		// 필드 초기화
		_label = Optional.empty();
		_operator = Optional.empty();
		_operands = new ArrayList<>();
		_comment = Optional.empty();
		_nixbpe = Optional.empty();

		// 입력 문자열이 비어있는 경우
		if (input.trim().isEmpty()) {
			throw new RuntimeException("This is an empty input string.");
		}

		// 주석 처리
		if (input.trim().startsWith(".")) {
			_comment = Optional.of(input.trim().substring(1).trim());
			return;
		}

		String[] parts = input.split("\\s+", 4);
		int index = 0;
		/**
		 * @brief 라벨 처리
 		 */
		if (parts.length > index && !instructionTable.search(parts[index]).isPresent()) {
			_label = Optional.of(parts[index]);
			index++;
		}
		/**
		 * @brief 연산자 혹은 지시어 처리
		 * @details 기계어 목록 테이블(_instTable)을 참조하여 유효한 연산자인지 확인
		 */
		if (parts.length > index) {
			String operator = parts[index];
			isFormat4 = operator.startsWith("+");
			// 4형식 명령어 처리
			if (isFormat4)
				operator = operator.substring(1); // '+' 제거
			// 지시어 혹은 기계어 목록에서 유효한 연산자인지 확인
			if (directives.contains(operator) || instructionTable.search(operator).isPresent()) {
				_operator = Optional.of(parts[index]);
				index++;
			}
		}
		/**
		 * @brief 피연산자 처리
		 */
		if (parts.length > index) {
			_operands.addAll(Arrays.asList(parts[index].split(",")));
			index++;
		}
		/**
		 * @brief comment 처리
		 */
		if (parts.length > index) {
			_comment = Optional.of(parts[index]);
		}

		// 각 명령어와 피연산자의 종류에 따라 nixbpe 결정
		setNixbpeBits();

		// 명령어 및 지시어에 따라 LOCCTR 계산
		if (_operator.isPresent()) {
			locctr[0] += calculateInstructionLength(_operator.get(), isFormat4, _operands);
			nextLocctr = locctr[0];
		}
	}

	// 명령어의 길이를 계산하기 위한 메소드
	private int calculateInstructionLength(String operator, boolean isFormat4, List<String> operands) {
		// 지시어인 경우
		if (directives.contains(operator)) {
			switch (operator) {
				case "RESB":
					return Integer.parseInt(operands.get(0));
				case "RESW":
					return 3 * Integer.parseInt(operands.get(0));
				case "BYTE":
					String operand = operands.get(0);
					if (operand.startsWith("C'"))
						return operand.length() - 3;
					else if (operand.startsWith("X'"))
						return (operand.length() - 3) / 2;
					break;
				case "WORD":
					return 3;
				case "LTORG":
					return 3;
			}
			return 0;
		}
		// 연산자인 경우
		Optional<InstructionInfo> instructionOpt = instructionTable.search(operator);
		if (!instructionOpt.isPresent()) {
			throw new RuntimeException("Instruction not found for operator: " + operator);
		}
		InstructionInfo instruction = instructionOpt.get();
		int length = instruction.getFormat();
		return isFormat4 ? length + 1 : length;
	}

	public static void setInstructionTable(InstructionTable instTable) {
		Token.instructionTable = instTable;  // 정적 메소드를 통한 InstructionTable 설정
	}

	private void setNixbpeBits() {
		// InstructionTable 사용
		if (instructionTable == null) {
			throw new IllegalStateException("The machine language list table has not been initialized.");
		}

		_nixbpe = Optional.of(0); // 초기화: 모든 비트가 0

		if (!_operator.isPresent()) return;

		String op = _operator.get();

		// E (extended format) bit 설정
		if (op.startsWith("+")) {
			op = op.substring(1); // '+' 기호 제거
			_nixbpe = Optional.of(_nixbpe.get() | 0x31); // n=1, i=1, e=1
			_operator = Optional.of(op);

		} else if (op.equals("RSUB")) {
			_nixbpe = Optional.of(0x30); // n=1, i=1
		} else {
			// Simple Addressing & PC-relative
			_nixbpe = Optional.of(_nixbpe.get() | 0x32); // n=1, i=1, p=1
		}

		Optional<InstructionInfo> instructionInfo = instructionTable.search(op);

		// 지시어인 경우 예외 처리 로직 추가
		if (!instructionInfo.isPresent()) return; // 명령어가 없는 경우

		InstructionInfo info = instructionInfo.get();

		// 피연산자가 지정되어 있는지 확인하고 그에 따라 처리
		for (int i = 0; i < _operands.size(); i++) {
			String operand = _operands.get(i);

			// Indirect Addressing: @
			if (operand.startsWith("@")) {
				_nixbpe = Optional.of(0x22); // n=1, i=0, p=1
				operand = operand.substring(1);
				_operands.set(0, operand);
			}

			// Immediate addressing: #
			if (operand.startsWith("#")) {
				_nixbpe = Optional.of(0x10); // n=0, i=1
				operand = operand.substring(1);
				_operands.set(0, operand);
			}

			// Indexed addressing: 마지막 피연산자가 "X"인 경우
			if (operand.equals("X") && i == _operands.size() - 1) {
				_nixbpe = Optional.of(_nixbpe.get() | 0x08); // x=1
			}
		}
	}

	// TODO: 필요한 getter 구현하기.
	// 라벨을 반환하는 getter
	public String getLabel() {
		return _label.orElse("");
	}

	// 연산자를 반환하는 getter
	public String getOperator() {
		return _operator.orElse("");
	}

	// 피연산자를 반환하는 getter
	public List<String> getOperands() {
		return _operands;
	}

	// 주석을 반환하는 getter
	public String getComment() {
		return _comment.orElse("");
	}

	// NIXBPE 비트 값을 반환하는 getter
	public int getNixbpe() {
		return _nixbpe.orElse(0);
	}

	// 현재 LOCCTR 값을 반환하는 getter
	public int getLocctr() {
		return this.currentLocctr;
	}

	// LOCCTR의 값을 증가시키는 메소드
	public void incrementLocctr(int increment) {
		this.nextLocctr += increment;
	}

	// 다음 라인의 LOCCTR 값을 반환하는 getter
	public int getNextLocctr() {
		return this.nextLocctr;
	}

	/**
	 * 토큰의 iNdirect bit가 1인지 여부를 반환한다.
	 * 
	 * @return N bit가 1인지 여부
	 */
	public boolean isN() {
		// TODO: 구현하기.
		return _nixbpe.isPresent() && (_nixbpe.get() & 0x20) != 0 && (_nixbpe.get() & 0x30) != 0x30;
	}

	/**
	 * 토큰의 Immediate bit가 1인지 여부를 반환한다.
	 * 
	 * @return I bit가 1인지 여부
	 */
	public boolean isI() {
		// TODO: 구현하기.
		return _nixbpe.isPresent() && (_nixbpe.get() & 0x10) != 0 && (_nixbpe.get() & 0x30) != 0x30;
	}

	/**
	 * 토큰의 indeX bit가 1인지 여부를 반환한다.
	 * 
	 * @return X bit가 1인지 여부
	 */
	public boolean isX() {
		// TODO: 구현하기.
		return _nixbpe.isPresent() && (_nixbpe.get() & 0x08) != 0;
	}

	/*
	 * Base relative는 구현하지 않음.
	 * public boolean isB() {
	 * return false;
	 * }
	 */

	/**
	 * 토큰의 Pc relative bit가 1인지 여부를 반환한다.
	 * 
	 * @return P bit가 1인지 여부
	 */
	public boolean isP() {
		// TODO: 구현하기.
		return _nixbpe.isPresent() && (_nixbpe.get() & 0x02) != 0;
	}

	/**
	 * 토큰의 Extra bit가 1인지 여부를 반환한다.
	 * 
	 * @return E bit가 1인지 여부
	 */
	public boolean isE() {
		// TODO: 구현하기.
		return _nixbpe.isPresent() && (_nixbpe.get() & 0x01) != 0;
	}

	/**
	 * 토큰을 String으로 변환한다. 원활한 디버깅을 위해 기본적으로 제공한 함수이며, Assembler.java에서는 해당 함수를 사용하지
	 * 않으므로 자유롭게 변경하여 사용한다.
	 * 아래 함수는 피연산자에 X가 지정되었더라도 _operands는 X를 저장하지 않고 X bit만 1로 변경한 상태를 가정하였다.
	 */
	@Override
	public String toString() {
		String label = _label.orElse("(no label)");
		String operator = (isE() ? "+" : "") + _operator.orElse("(no operator)");
		String operand = (isN() ? "@" : "") + (isI() ? "#" : "")
				+ (_operands.isEmpty() ? "(no operand)" : _operands.stream().collect(Collectors.joining(",")))
				+ (isX() ? (_operands.isEmpty() ? "X" : "/X") : "");
		String comment = _comment.orElse("(no comment)");
		return label + '\t' + operator + '\t' + operand + '\t' + comment;
	}

	/** label */
	private Optional<String> _label;

	/** operator */
	private Optional<String> _operator;

	/** operand */
	private ArrayList<String> _operands;

	/** comment */
	private Optional<String> _comment;

	/** nixbpe 비트를 저장하는 변수 */
	private Optional<Integer> _nixbpe;

	// 기계어 목록 테이블(_instTable) 참조
	private static InstructionTable instructionTable;

	// locctr 배열 참조
	private int[] locctr;

	// 현재 토큰의 시작 locctr
	private int currentLocctr;

	// 다음 라인의 locctr
	private int nextLocctr;

	// 지시어 목록을 HashSet으로 정의
	static final Set<String> directives = new HashSet<>(Set.of(
			"START", "END", "EXTDEF", "EXTREF", "LTORG",
			"RESB", "RESW", "CSECT", "EQU", "BYTE", "WORD"
	));
}
