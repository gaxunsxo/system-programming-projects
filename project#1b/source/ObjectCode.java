import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ObjectCode {
	public ObjectCode() {
		// TODO: 초기화.
		this.headerRecord = new StringBuilder();
		this.defineRecord = new StringBuilder();
		this.referenceRecord = new StringBuilder();
		this.textRecords = new StringBuilder();
		this.modificationRecords = new StringBuilder();
		this.endRecord = new StringBuilder();
	}

	// 레코드 작성 메소드
	public void appendHeader(String name, int startAddress, int length) {
		headerRecord.append(String.format("H%-6s%06X%06X\n", name, startAddress, length));
	}

	public void appendTextRecord(int startAddress, String objectCode, int length) {
		textRecords.append(String.format("T%06X%02X%s\n", startAddress, length, objectCode));
	}

	public void appendDefineRecord(Map<String, Integer> symbols) {
		defineRecord.append("D");
		symbols.forEach((symbol, address) -> {
			defineRecord.append(String.format("%-6s%06X", symbol, address));
		});
		defineRecord.append("\n");
	}

	public void appendReferenceRecord(List<String> symbols) {
		referenceRecord.append("R");
		symbols.forEach(symbol -> {
			referenceRecord.append(String.format("%-6s", symbol));
		});
		referenceRecord.append("\n");
	}

	public void appendModificationRecords(List<Token> tokens, SymbolTable symbolTable) {
		for (Token token : tokens) {
			// WORD 지시어 또는 4형식 명령어에 대한 처리
			if (token.getOperator().equals("WORD") || token.isE()) {
				String operands = token.getOperands().get(0);
				// 연산자 '+' 또는 '-'로 분리된 부분 처리
				Matcher matcher = Pattern.compile("([+-]?\\b\\w+\\b)").matcher(operands);

				// 주소 계산을 위한 초기 위치 설정
				int addressOffset = (token.isE() ? 1 : 0); // E 형식일 때는 주소 시작을 다르게 조정

				while (matcher.find()) {
					String symbol = matcher.group(1).trim();
					char sign = symbol.charAt(0) == '+' || symbol.charAt(0) == '-' ? symbol.charAt(0) : '+';
					symbol = symbol.replace("+", "").replace("-", ""); // 심볼에서 연산자 제거

					if (symbolTable.searchSymbol(symbol).isPresent()) { // 심볼 테이블에서 심볼을 검색
						int modificationLength = token.isE() ? 5 : 6;
						modificationRecords.append(String.format("M%06X%02X%c%s\n",
								token.getLocctr() + addressOffset, modificationLength, sign, symbol));
					}
				}
			}
		}
	}

	public void appendEndRecord(List<Token> tokens) {
		for (Token token : tokens) {
			if (token.getOperator().equals("START")) {
				endRecord.append(String.format("E%06X\n",token.getLocctr()));
			} else if (token.getOperator().equals("CSECT")) {
				endRecord.append(String.format("E\n"));
			}
		}
	}

	/**
	 * ObjectCode 객체를 String으로 변환한다. Assembler.java에서 오브젝트 코드를 출력하는 데에 사용된다.
	 */
	@Override
	public String toString() {
		// TODO: toString 구현하기.
		return headerRecord.toString() +
				defineRecord.toString() +
				referenceRecord.toString() +
				textRecords.toString() +
				modificationRecords.toString() +
				endRecord.toString();
	}

	// TODO: private field 선언.
	private StringBuilder headerRecord;
	private StringBuilder defineRecord;
	private StringBuilder referenceRecord;
	private StringBuilder textRecords;
	private StringBuilder modificationRecords;
	private StringBuilder endRecord;
}
