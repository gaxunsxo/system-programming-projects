import java.util.StringTokenizer;

public class InstructionInfo {
	/**
	 * 기계어 목록 파일의 한 줄을 읽고, 이를 파싱하여 저장한다.
	 * 
	 * @param line 기계어 목록 파일의 한 줄
	 * @throws RuntimeException 잘못된 파일 형식
	 */
	public InstructionInfo(String line) throws RuntimeException {
		// TODO: line을 파싱하여 _name, _opcode, _numberOfOperand, _format 초기화하기.
		StringTokenizer st = new StringTokenizer(line);
		try {
			// name - format - opcode - numberOfOperand
			_name = st.nextToken();
			_format = Integer.parseInt(st.nextToken());
			_opcode = Integer.parseInt(st.nextToken(), 16);
			_numberOfOperand = Integer.parseInt(st.nextToken());
		} catch (Exception e) {
			throw new RuntimeException("Error parsing instrction data: " + e.getMessage());
		}
	}

	/**
	 * 기계어 명칭을 반환한다.
	 * 
	 * @return 기계어 명칭
	 */
	public String getName() {
		return _name;
	}

	/**
	 * 기계어의 opcode를 반환한다.
	 * 
	 * @return 기계어의 opcode
	 */
	public int getOpcode() {
		return _opcode;
	}

	/**
	 * 기계어의 operand 개수를 반환한다.
	 * 
	 * @return 기계어의 operand 개수
	 */
	public int getNumberOfOperand() {
		return _numberOfOperand;
	}

	/**
	 * 기계어의 형식을 반환한다.
	 * 
	 * @return 기계어의 형식. 1: ~~, 2: ~~, 3: ~~, 4:~~, 5:~~ (본인의 사용 방식에 맞춰 작성한다)
	 */
	public int getFormat() {
		return _format;
	}

	private String _name;
	private int _opcode;
	private int _numberOfOperand;
	private int _format;
}