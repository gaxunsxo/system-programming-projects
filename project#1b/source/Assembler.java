import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.stream.Collectors;

/**
 * SIC/XE 머신을 위한 Assembler 프로그램의 메인 루틴이다.
 * 
 * 작성 중 유의 사항
 * 1) Assembler.java 파일의 기존 코드를 변경하지 말 것. 다른 소스 코드의 구조는 본인의 편의에 따라 변경해도 됨.
 * 2) 새로운 클래스, 새로운 필드, 새로운 메소드 선언은 허용됨. 단, 기존의 필드와 메소드를 삭제하거나 대체하는 것은 불가함.
 * 3) 예외 처리, 인터페이스, 상속 사용 또한 허용됨.
 * 4) 파일, 또는 콘솔창에 한글을 출력하지 말 것. (채점 상의 이유)
 * 
 * 제공하는 프로그램 구조의 개선점을 제안하고 싶은 학생은 보고서의 결론 뒷부분에 첨부 바람. 내용에 따라 가산점이 있을 수 있음.
 */
public class Assembler {
	public static void main(String[] args) {
		try {
			Assembler assembler = new Assembler("inst_table.txt");
			ArrayList<String> input = assembler.readInputFromFile("input.txt");
			ArrayList<ArrayList<String>> dividedInput = assembler.divideInput(input);
			dividedInput.size();

			ArrayList<ControlSection> controlSections = (ArrayList<ControlSection>) dividedInput.stream()
					.map(x -> assembler.pass1(x))
					.collect(Collectors.toList());

			String symbolsString = controlSections.stream()
					.map(x -> x.getSymbolString())
					.collect(Collectors.joining("\n\n"));
			String literalsString = controlSections.stream()
					.map(x -> x.getLiteralString())
					.collect(Collectors.joining("\n\n"));

			assembler.writeStringToFile("output_symtab.txt", symbolsString);
			assembler.writeStringToFile("output_littab.txt", literalsString);

			ArrayList<ObjectCode> objectCodes = (ArrayList<ObjectCode>) controlSections.stream()
					.map(x -> assembler.pass2(x))
					.collect(Collectors.toList());

			String objectCodesString = objectCodes.stream()
					.map(x -> x.toString())
					.collect(Collectors.joining("\n\n"));

			assembler.writeStringToFile("output_objectcode.txt", objectCodesString);
		} catch (Exception e) {
			System.out.println("Error : " + e.getMessage());
		}
	}

	public Assembler(String instFile) throws FileNotFoundException, IOException {
		_instTable = new InstructionTable(instFile);
	}

	private ArrayList<ArrayList<String>> divideInput(ArrayList<String> input) {
		ArrayList<ArrayList<String>> divided = new ArrayList<ArrayList<String>>();
		String lastStr = input.get(input.size() - 1);

		ArrayList<String> tmpInput = new ArrayList<String>();
		for (String str : input) {
			if (str.contains("CSECT")) {
				if (!tmpInput.isEmpty()) {
					tmpInput.add(lastStr);
					divided.add(tmpInput);
					tmpInput = new ArrayList<String>();
					tmpInput.add(str);
				}
			} else {
				tmpInput.add(str);
			}
		}

		if (!tmpInput.isEmpty()) {
			divided.add(tmpInput);
		}

		return divided;
	}

	private ArrayList<String> readInputFromFile(String inputFileName) throws FileNotFoundException, IOException {
		ArrayList<String> input = new ArrayList<String>();

		File file = new File(inputFileName);
		BufferedReader bufReader = new BufferedReader(new FileReader(file));

		String line = "";
		while ((line = bufReader.readLine()) != null)
			input.add(line);

		bufReader.close();

		return input;
	}

	private void writeStringToFile(String fileName, String content) throws IOException {
		File file = new File(fileName);

		BufferedWriter writer = new BufferedWriter(new FileWriter(file));
		writer.write(content);
		writer.close();
	}

	private ControlSection pass1(ArrayList<String> input) throws RuntimeException {
		return new ControlSection(_instTable, input);
	}

	private ObjectCode pass2(ControlSection controlSection) throws RuntimeException {
		return controlSection.buildObjectCode();
	}

	private InstructionTable _instTable;
}