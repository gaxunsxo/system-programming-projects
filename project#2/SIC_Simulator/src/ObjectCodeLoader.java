import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.io.IOException;

public class ObjectCodeLoader {
	private ResourceManager resourceManager;
	private String currentSection; // 현재 섹션을 추적하는 변수 
	private List<ModificationRecord> modificationRecords; // 수정 레코드를 저장할 리스트
	private int progLength = 0;
	private int address = 0;

    public ObjectCodeLoader(ResourceManager resourceManager) {
        this.resourceManager = resourceManager;
        this.modificationRecords = new ArrayList<>(); // 수정 레코드 리스트 초기화 
    }
    
    // 수정 레코드 클래스 정의
    private static class ModificationRecord {
        int startAddress;
        int length;
        String operation;
        String symbol;
        String controlSection;

        public ModificationRecord(int startAddress, int length, String operation, String symbol, String controlSection) {
            this.startAddress = startAddress;
            this.length = length;
            this.operation = operation;
            this.symbol = symbol;
            this.controlSection = controlSection;
        }
    }

    public void load(String filePath) throws IOException {
    	 try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
             String line;
             while ((line = br.readLine()) != null) {
                 switch (line.charAt(0)) {
                     case 'H':
                         handleHeaderRecord(line);
                         break;
                     case 'D':
                    	 handleDefineRecord(line);
                    	 break;
                     case 'R':
                    	 handleReferenceRecord(line);
                    	 break;
                     case 'T':
                         handleTextRecord(line);
                         break;
                     case 'M':
                         handleModificationRecord(line);
                         break;
                     case 'E':
                         handleEndRecord(line);
                         break;
                     default:
                         throw new IllegalArgumentException("Unknown record type: " + line.charAt(0));
                 }
             }
             processModificationRecords(); // 전체 로드가 끝난 후 수정 레코드 처리
    	 }
    }
    
    private void handleHeaderRecord(String line) {
    	// 각 컨트롤 섹션 별로 분리해서 저장하기 
        String programName = line.substring(1, 7).trim();
        String startAddress = line.substring(7, 13).trim();
        String programLength = line.substring(13, 19).trim();

        currentSection = programName;
        resourceManager.setCurrentControlSection(currentSection);
        resourceManager.setProgname(programName, currentSection);
        resourceManager.setProgLength(programLength, currentSection);
        resourceManager.setStartADDR(startAddress, currentSection);
        
        address += progLength;
        
        // symbol table 등록
        resourceManager.symtabList.putSymbol(programName, address);
        
        // 실제 로드 주소 업데이트
        progLength = Integer.parseInt(programLength, 16);
        resourceManager.updateCurrentLoadAddress(progLength);
    }
    
    private void handleDefineRecord(String line) {
    	// D 레코드 처리 로직 (심볼 테이블 등록)
    	// 
        int length = line.length();
        for (int i = 1; i < length; i += 12) {
            String symbol = line.substring(i, i + 6).trim();
            String address = line.substring(i + 6, i + 12).trim();
            int addressInt = Integer.parseInt(address, 16);
            resourceManager.symtabList.putSymbol(symbol, addressInt);
        } 	
    }
    
    private void handleReferenceRecord(String line) {
    }
    
    private void handleTextRecord(String line) {
        String startAddress = line.substring(1, 7).trim();
        String length = line.substring(7, 9).trim();
        String data = line.substring(9).trim();

        int address = Integer.parseInt(startAddress, 16);
        int byteCount = Integer.parseInt(length, 16);
        
        // 현재 섹션의 시작 주소 가져오기
        int sectionStartAddress = resourceManager.getProgramCounter(currentSection);
        
        for (int i = 0; i < byteCount * 2; i += 2) {
            String byteStr = data.substring(i, i + 2);
            int value = Integer.parseInt(byteStr, 16);
            resourceManager.setMemory(sectionStartAddress + address, value);
            address++;
        }   
    }
    
    private void handleModificationRecord(String line) {
    	 String startAddress = line.substring(1, 7).trim();
         int length = Integer.parseInt(line.substring(7, 9).trim());
         String op = line.substring(9, 10).trim();
         String symbol = line.substring(10).trim();

         int address = Integer.parseInt(startAddress, 16);
         String currentSection = resourceManager.getCurrentControlSection();
         ModificationRecord record = new ModificationRecord(address, length, op, symbol, currentSection);
         modificationRecords.add(record);
    }
    
    private void handleEndRecord(String line) {
        String firstInstructionAddress = line.substring(1).trim();
        resourceManager.setFirstInstructionAddress(firstInstructionAddress);
    }
    
    private void processModificationRecords() {
        for (ModificationRecord record : modificationRecords) {
            int address = record.startAddress;
            int length = record.length;
            String op = record.operation;
            String symbol = record.symbol;
            String controlSection = record.controlSection;

            int sectionStartAddress = resourceManager.getProgramCounter(controlSection);
            int fullAddress = sectionStartAddress + address;

            // 심볼 테이블에서 심볼의 주소를 찾음
            Integer symbolAddressObj = resourceManager.symtabList.getSymbol(symbol);
            if (symbolAddressObj == null) {
                throw new IllegalArgumentException("Symbol not found: " + symbol);
            }
            int symbolAddress = symbolAddressObj;
            
            // 기존 메모리 값을 읽어옴 (반 바이트 단위로 주어진 길이만큼)
            int currentValue = 0;
            int byteCount = (length + 1) / 2;  // 반 바이트 단위로 주어진 길이 => 바이트 단위로 변환
            for (int i = 0; i < byteCount; i++) {
                currentValue <<= 8;
                currentValue |= resourceManager.getMemory(fullAddress + i) & 0xFF;
            }

            // 현재 메모리 값을 연산에 맞춰 조정 (부호 연산 처리)
            int newValue = currentValue;
            if (op.equals("+")) {
                newValue += symbolAddress;
            } else if (op.equals("-")) {
                newValue -= symbolAddress;
            } else {
                throw new IllegalArgumentException("Unknown operation: " + op);
            }

            // 새로운 값을 메모리에 저장 
            for (int i = 0; i < byteCount; i++) {
                int byteToWrite = (newValue >> ((byteCount - 1 - i) * 8)) & 0xFF;
                resourceManager.setMemory(fullAddress + i, byteToWrite);
            }
        }
    }   
}


