import java.util.*;

public class ResourceManager {
	private int[] memory;
    private int[] registers;
    private char conditionCode; // 조건 코드
    private Map<String, ControlSection> controlSections;
    private String currentControlSection;
    public SymbolTable symtabList;
    private Map<String, Integer> programCounters; // 각 컨트롤 섹션의 프로그램 카운터 저장
    private int currentLoadAddress; // 현재 로드 주소를 저장
    private int programStartAddress; // 전체 프로그램의 시작 주소
    private int totalProgramLength; // 전체 프로그램의 길이
    private String totalProgramName; // 전체 프로그램의 이름
    private Integer activeDevice; // 현재 활성화된 장치
    private Integer activeDeviceAddress; // 현재 활성화된 장치의 주소 
    private Set<Integer> previouslyActivatedDevices; // 이전에 활성화된 장치들
    private int targetAddress; // 타겟 주소 저장

    public ResourceManager() {
        memory = new int[0x100000]; // 1MB의 메모리 
        for (int i = 0; i < memory.length; i++) {
            memory[i] = 0xFF; // 메모리를 0xFF로 초기화
        }
        registers = new int[10]; // 10개의 범용 레지스터
        conditionCode = ' '; // 초기 조건 코드
        controlSections = new HashMap<>();
        symtabList = new SymbolTable();
        programCounters = new HashMap<>(); // 프로그램 카운터 맵 초기화
        currentLoadAddress = 0; // 프로그램의 실제 시작 주소로 초기화해야 함
        activeDevice = null;
        activeDeviceAddress = null;
        previouslyActivatedDevices = new HashSet<>();
    }
    
    public void setProgname(String progname, String section) {
        // 프로그램 이름 설정 구현
    	getOrCreateControlSection(section).progName = progname;
    }

    public void setProgLength(String length, String section) {
        // 프로그램 길이 설정 구현
    	getOrCreateControlSection(section).progLength = length;
    }

    public void setStartADDR(String address, String section) {
    	 // 시작 주소 설정 구현
        ControlSection controlSection = getOrCreateControlSection(section);
        controlSection.startAddress = address;
        // 섹션의 실제 시작 주소 계산
        controlSection.loadAddress = currentLoadAddress;
        programCounters.put(section, currentLoadAddress);
        
        // 전체 프로그램의 시작 주소 설정 (첫 번째 섹션의 주소 사용)
        if (currentLoadAddress == 0) {
            programStartAddress = Integer.parseInt(address, 16);
            totalProgramName = controlSection.progName;
            //totalProgramLength = Integer.parseInt(controlSection.progLength, 16);
        }

        totalProgramLength += Integer.parseInt(controlSection.progLength, 16);
    }

    // 전체 프로그램의 이름을 반환
    public String getTotalProgName() {
        return totalProgramName;  
    }

    // 전체 프로그램의 길이를 반환
    public int getTotalProgLength() {
        return totalProgramLength;
    }

    // 전체 프로그램의 시작 주소를 반환
    public int getTotalProgStartAddress() {
        return programStartAddress;
    }
    
    public String getStartADDR(String section) {
    	return getOrCreateControlSection(section).startAddress;
    }
    
    public void setFirstInstructionAddress(String address) {
        // 첫 번째 명령어 주소 설정 구현
    	getOrCreateControlSection(currentControlSection).firstInstructionAddress = address;
    	
    }

    public void setCurrentControlSection(String section) {
    	currentControlSection = section;
    }
    
    public String getCurrentControlSection() {
    	return this.currentControlSection;
    }
    
    
    public ControlSection getOrCreateControlSection(String section) {
    	return controlSections.computeIfAbsent(section, k -> new ControlSection());
    }
    
    public void setMemory(int address, int value) {
        memory[address] = value;
    }
    
    public int getMemory(int address) {
        return memory[address]; // 주어진 주소에 있는 값을 반환 
    }
    
    public int getMemorySize() {
        return memory.length;
    }
    
    // Register 관리 
    public void setRegister(int index, int value) {
        registers[index] = value;
    }

    public int getRegister(int index) {
        return registers[index];
    }
    
    // Device 관리 
    public void setDeviceActive(int address) {
        int device = getMemory(address);
        if (activeDevice != null && activeDeviceAddress == address) {
            // 이미 사용 중인 주소인 경우
        } else { 
            // Activate the new device
            activeDevice = device;
            activeDeviceAddress = address; // 장치 번호와 주소를 함께 저장
        }
    }

    public boolean wasDevicePreviouslyActivated(int address) {
        return previouslyActivatedDevices.contains(address);
    }

    public void setDeviceInactive(int address) {
        if (activeDevice != null && activeDeviceAddress == address) {
            activeDevice = null;
            activeDeviceAddress = -1; // 장치 주소도 제거
        }
    }
    
    public boolean isDeviceActive(int address) {
        return activeDevice != null && activeDeviceAddress == address;
    }
    
    public Integer getActiveDevice() {
        return activeDevice;
    }
    
    public int getDeviceAddress(int address) {
        return activeDeviceAddress != -1 ? activeDeviceAddress : -1; // 장치 주소 반환, 없으면 -1 반환
    }

    public int readDataFromDevice(int address) {
        if (isDeviceActive(address)) {
            return getMemory(address);
        }
        throw new IllegalArgumentException("Device not active or offset out of bounds");
    }
    
    // CC 값 관리 
    public char getConditionCode() {
        return conditionCode;
    }

    public void setConditionCode(char conditionCode) {
        this.conditionCode = conditionCode;
    }

    // Target Address 관리
    public int getTargetAddress() {
        return targetAddress;
    }

    public void setTargetAddress(int targetAddress) {
        this.targetAddress = targetAddress;
    }
    

    // Program Counter 관리
    public int getProgramCounter(String section) {
        return programCounters.getOrDefault(section, 0);
    }

    public void setProgramCounter(String section, int address) {
        programCounters.put(section, address);
    }

    public void updateCurrentLoadAddress(int length) {
        currentLoadAddress += length;
    }
    
    public int getProgramStartAddress() {
    	return programStartAddress;
    }

    public int getFirstInstructionAddress() {
    	return Integer.parseInt(getOrCreateControlSection(currentControlSection).firstInstructionAddress, 16);
    }
    
    private static class ControlSection {
        String progName;
        String progLength;
        String startAddress;
        String firstInstructionAddress;
        int loadAddress; // 실제 메모리 로드 주소
    }
}

class SymbolTable {
	private Map<String, Integer> symbols = new HashMap<>();
	
    public void putSymbol(String symbol, int address) {
        // Implementation to add symbol to the symbol table
    	symbols.put(symbol, address);
    }
    
    public Integer getSymbol(String symbol) {
    	return symbols.get(symbol);
    }
    
}