import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.Optional;

public class SymbolTable {
	/**
	 * 심볼 테이블 객체를 초기화한다.
	 */
	public SymbolTable() {
		symbolMap = new LinkedHashMap<String, Symbol>();
	}

	/**
	 * EQU를 제외한 명령어/지시어에 label이 포함되어 있는 경우, 해당 label을 심볼 테이블에 추가한다.
	 * 
	 * @param label   라벨
	 * @param address 심볼의 주소
	 * @throws RuntimeException (TODO: exception 발생 조건을 작성하기)
	 */
	public void putLabel(String label, int address) throws RuntimeException {
		// TODO: EQU를 제외한 명령어/지시어의 label로 생성되는 심볼을 추가하기.
		if (label == null || label.trim().isEmpty()) {
			throw new RuntimeException("Label cannot be null or empty.");
		}
		if (!symbolMap.containsKey(label)) {
			symbolMap.put(label, new Symbol(label, address));
		}
	}

	/**
	 * EQU에 label이 포함되어 있는 경우, 해당 label을 심볼 테이블에 추가한다.
	 * 
	 * @param label    라벨
	 * @param locctr   locctr 값
	 * @param equation equation 문자열
	 * @throws RuntimeException equation 파싱 오류
	 */
	public void putLabel(String label, int locctr, String equation) throws RuntimeException {
		// TODO: EQU의 label로 생성되는 심볼을 추가하기.
		symbolMap.put(label, new Symbol(label, calculateEquation(locctr, equation)));
	}

	private int calculateEquation(int locctr, String equation) throws RuntimeException {
		// 연산자와 피연산자를 분리하기 위한 정규 표현식 패턴
		Pattern pattern = Pattern.compile("([\\w*]+)|([+-])");
		Matcher matcher = pattern.matcher(equation);
		List<String> parts = new ArrayList<>();

		while (matcher.find()) {
			parts.add(matcher.group());
		}

		int result = 0;

		try {
			for (int i = 0; i < parts.size(); i++) {
				String part = parts.get(i).trim();

				if (part.equals("*")) {  // 현재 LOCCTR 값
					result = locctr;
				} else if (symbolMap.containsKey(part)) {  // 심볼 주소 참조
					int symbolAddress = symbolMap.get(part).getAddress();
					if (i > 0 && "+".equals(parts.get(i-1).trim())) {
						result += symbolAddress;
					} else if (i > 0 && "-".equals(parts.get(i-1).trim())) {
						result -= symbolAddress;
					} else {
						result = symbolAddress;
					}
				} else if (part.matches("\\d+")) {  // 숫자 처리
					int value = Integer.parseInt(part);
					if (i > 0 && "+".equals(parts.get(i-1).trim())) {
						result += value;
					} else if (i > 0 && "-".equals(parts.get(i-1).trim())) {
						result -= value;
					} else {
						result = value;
					}
				} else if (!"+".equals(part) && !"-".equals(part) && !part.isEmpty()) {
					throw new IllegalArgumentException("Invalid equation component: " + part);
				}
			}
		} catch (NumberFormatException e) {
			throw new RuntimeException("Error parsing equation: " + equation, e);
		}

		return result;
	}


	/**
	 * EXTREF에 operand가 포함되어 있는 경우, 해당 operand를 심볼 테이블에 추가한다.
	 * 
	 * @param refer operand에 적힌 하나의 심볼
	 * @throws RuntimeException (TODO: exception 발생 조건을 작성하기)
	 */
	public void putRefer(String refer) throws RuntimeException {
		// TODO: EXTREF의 operand로 생성되는 심볼을 추가하기.
		if (!symbolMap.containsKey(refer)) {
			symbolMap.put(refer, new Symbol(refer, -1)); // 주소: -1로 설정
		}
	}

	/**
	 * 심볼 테이블에서 심볼을 찾는다.
	 * 
	 * @param name 찾을 심볼 명칭
	 * @return 심볼. 없을 경우 empty
	 */
	public Optional<Symbol> searchSymbol(String name) {
		// TODO: symbolMap에서 name에 해당하는 심볼을 찾아 반환하기.
		return Optional.ofNullable(symbolMap.get(name));
	}

	/**
	 * 심볼 테이블에서 심볼을 찾아, 해당 심볼의 주소를 반환한다.
	 * 
	 * @param symbolName 찾을 심볼 명칭
	 * @return 심볼의 주소. 없을 경우 empty
	 */
	public Optional<Integer> getAddress(String symbolName) {
		Optional<Symbol> optSymbol = searchSymbol(symbolName);
		return optSymbol.map(s -> s.getAddress());
	}

	/**
	 * 심볼 테이블을 String으로 변환한다. Assembler.java에서 심볼 테이블을 출력하기 위해 사용한다.
	 */
	@Override
	public String toString() {
		// TODO: 심볼 테이블을 String으로 표현하기. Symbol 객체의 toString을 활용하자.
		StringBuilder builder = new StringBuilder();
		for (Map.Entry<String, Symbol> entry : symbolMap.entrySet()) {
			Symbol symbol = entry.getValue();
			builder.append(symbol.toString());
			builder.append(System.lineSeparator());
		}
		return builder.toString();
	}

	/** 심볼 테이블. key: 심볼 명칭, value: 심볼 객체 */
	private LinkedHashMap<String, Symbol> symbolMap;
}

class Symbol {
	/**
	 * 심볼 객체를 초기화한다.
	 * 
	 * @param name    심볼 명칭
	 * @param address 심볼의 절대 주소
	 */
	public Symbol(String name, int address /* , 추가로 선언한 field들... */) {
		// TODO: 심볼 객체 초기화.
		_name = name;
		_address = address;
	}

	/**
	 * 심볼 명칭을 반환한다.
	 * 
	 * @return 심볼 명칭
	 */
	public String getName() {
		return _name;
	}

	/**
	 * 심볼의 주소를 반환한다.
	 * 
	 * @return 심볼 주소
	 */
	public int getAddress() {
		return _address;
	}

	// TODO: 추가로 선언한 field에 대한 getter 작성하기.

	/**
	 * 심볼을 String으로 변환한다.
	 */
	@Override
	public String toString() {
		// TODO: 심볼을 String으로 표현하기.
		if (_address == -1) { // 주소가 -1인 경우 외부 참조 심볼
			return _name + "\tREF";
		} else {
			return _name + "\t" + String.format("%04X", _address);
		}
	}

	/** 심볼의 명칭 */
	private String _name;

	/** 심볼의 주소 */
	private int _address;

	// TODO: 추가로 필요한 field 선언
}