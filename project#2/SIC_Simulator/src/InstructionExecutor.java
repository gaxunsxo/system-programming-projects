public class InstructionExecutor {
	private ResourceManager resourceManager;
	private String currentSection;
    private int programCounter;
    private String lastObjectCode; // 마지막 실행된 오브젝트 코드
    private String currentInstructionName; // 현재 실행 중인 명령어
    
    public InstructionExecutor(ResourceManager resourceManager) {
    	this.resourceManager = resourceManager;
        this.programCounter = resourceManager.getProgramStartAddress(); // 초기 프로그램 카운터를 전체 프로그램의 시작 주소로 설정
        this.lastObjectCode = ""; // 초기화
        this.currentSection = resourceManager.getCurrentControlSection(); // 현재 섹션 설정
    }

    // 타겟 주소를 계산하는 메서드
    private int calculateTargetAddress(DecodedInstruction decodedInstruction) {
        int TA = 0;
        int disp = decodedInstruction.getDisp();

        if (decodedInstruction.iseFlag()) {
            // Extended Addressing
            TA = disp;
        } else if (decodedInstruction.ispFlag()) {
            // Simple Addressing
            TA = disp + programCounter + decodedInstruction.getLength();
        } else if (!decodedInstruction.ispFlag() && decodedInstruction.isnFlag()) {
            // Indirect Addressing
            TA = convertMemoryToInteger(disp, 3);
            TA = convertMemoryToInteger(TA, 3);
        }

        resourceManager.setTargetAddress(TA);
        return TA;
    }

    // 다음 명령어가 있는지 확인하는 메서드
    public boolean hasNextInstruction() {
        return programCounter < resourceManager.getMemorySize();
    }

    public void executeNextInstruction() {
    	while (resourceManager.getMemory(programCounter) == 0xFF && programCounter < resourceManager.getMemorySize()) {
            programCounter++;
        }

        if (programCounter >= resourceManager.getMemorySize()) {
            return; // 더 이상 실행할 명령어가 없음
        }
        
    	// 현재 Program Counter에 해당하는 값을 읽어옴
    	int value = resourceManager.getMemory(programCounter);
    	
    	// value의 상위 6비트를 추출하여 opcode 확인 
    	int opcode = value & 0xFC;
        Instruction instruction = Instruction.getInstruction(opcode);

        DecodedInstruction decodedInstruction = new DecodedInstruction(0, false, false, false, false, false, false, 0, 0);

        if (instruction == null) {
            if (value == 0x45) {
                decodedInstruction = new DecodedInstruction(0x45, false, false, false, false, false, false, 0, 3);
            } else if (value == 0xF1 || value == 0x05) {
                decodedInstruction = new DecodedInstruction(value, false, false, false, false, false, false, 0, 1);
            } else {
                // 프로그램 수행 종료
                System.out.println("Unknown opcode at address: " + programCounter);
                programCounter++;
            }
        } else {
            decodedInstruction = InstructionDecoder.decode(programCounter, resourceManager);
        }

        int instructionLength = decodedInstruction.getLength();
        StringBuilder objectCodeBuilder = new StringBuilder();
        for (int i = 0; i < instructionLength; i++) {
            objectCodeBuilder.append(String.format("%02X", resourceManager.getMemory(programCounter + i)));
        }
        lastObjectCode = objectCodeBuilder.toString();
        this.currentInstructionName = instruction.getName();

        // 명령어 실행
        switch (instruction.getOpcode()) {
        	case 0x14: // STL
        		executeSTL(decodedInstruction);
        		break;
        	case 0x48: // JSUB
        		executeJSUB(decodedInstruction);
        		break;
        	case 0x74: // LDT
        		executeLDT(decodedInstruction);
        		break;
        	case 0xE0: // TD
        		executeTD(decodedInstruction);
        		break;
        	case 0x30: // JEQ
        		executeJEQ(decodedInstruction);
        		break;
        	case 0xD8: // RD
        		executeRD(decodedInstruction);
        		break;
        	case 0xA0: // COMPR
        		executeCOMPR(decodedInstruction);
        		break;
        	case 0x50: // LDCH
        		executeLDCH(decodedInstruction);
        		break;
        	case 0x54: // STCH
        		executeSTCH(decodedInstruction);
        		break;
        	case 0xB8: // TIXR
        		executeTIXR(decodedInstruction);
        		break;
        	case 0x38: // JLT
        		executeJLT(decodedInstruction);
        		break;
        	case 0x10: // STX
        		executeSTX(decodedInstruction);
        		break;
        	case 0x4C: // RSUB
        		executeRSUB(decodedInstruction);
        		break;
            case 0x00: // LDA
                executeLDA(decodedInstruction);
                break;
            case 0x28: // COMP
            	executeCOMP(decodedInstruction);
            	break;
            case 0xDC: // WD
            	executeWD(decodedInstruction);
            	break;
            case 0x3C: // J
            	executeJ(decodedInstruction);
            	break;
            case 0x0C: // STA
                executeSTA(decodedInstruction);
                break;
            case 0xB4: // CLEAR
                executeCLEAR(decodedInstruction);
                break;
            // 다른 명령어들에 대한 처리 추가
            default:
                System.out.println("Unsupported instruction: " + instruction.getName());
                break;
        }
        
        // 현재 섹션의 프로그램 카운터 갱신
        resourceManager.setProgramCounter(currentSection, programCounter);
    }

    public String getLastObjectCode() {
        return lastObjectCode;
    }

     // 현재 명령어의 이름을 반환하는 메서드
     public String getCurrentInstructionName() {
        return currentInstructionName;
    }

    public int getProgramCounter() {
        return programCounter;
    }
    
    // STL 명령어 실행 
    private void executeSTL(DecodedInstruction decodedInstruction) {
    	// STL m : m .. m+2  <- (L)
    	int TA = calculateTargetAddress(decodedInstruction);
        int value = resourceManager.getRegister(2);
        convertIntegerToMemory(TA, value, 3);
        programCounter += decodedInstruction.getLength();
    }
    
    // JSUB 명령어 실행 
    private void executeJSUB(DecodedInstruction decodedInstruction) {
    	// JSUB m : L <- (PC); PC <- m 
    	// PC의 값을 L 레지스터에 저장하고 PC의 값을 TA로 변경 
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setRegister(2, programCounter + decodedInstruction.getLength());
        programCounter = TA;
    }
    
    // LDT 명령어 실행
    private void executeLDT(DecodedInstruction decodedInstruction) {
    	// LDT m : T <- (m .. m+2)
    	int TA = calculateTargetAddress(decodedInstruction);
        int value = convertMemoryToInteger(TA, 3);
        resourceManager.setRegister(5, value);
        programCounter += decodedInstruction.getLength();
    }
    
    // TD 명령어 실행
    private void executeTD(DecodedInstruction decodedInstruction) {
    	// TD m : Test device specified by (m)
        int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setDeviceActive(TA);
        if (resourceManager.isDeviceActive(TA)) {
            resourceManager.setConditionCode('<');
        } else {
            resourceManager.setConditionCode('=');
        }
        programCounter += decodedInstruction.getLength();
    }
    
    // JEQ 명령어 실행
    private void executeJEQ(DecodedInstruction decodedInstruction) {
    	// JEQ m : PC <- m if CC set to = 
    	int TA = calculateTargetAddress(decodedInstruction);
        if (resourceManager.getConditionCode() == '=') {
            programCounter = TA;
        } else {
            programCounter += decodedInstruction.getLength();
        }
    }
    
    // RD 명령어 실행
    private void executeRD(DecodedInstruction decodedInstruction) {
    	// RD m : A[rightmost byte] <- data from device specified by (m)
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);

        // 디바이스의 데이터를 0으로 초기화
        resourceManager.setMemory(TA, 0);
        
        // 장치에서 데이터 읽어오기
        int data = resourceManager.readDataFromDevice(TA);

        // 레지스터 A의 오른쪽 바이트에 데이터 저장
        int registerA = resourceManager.getRegister(0);
        registerA = (registerA & 0xFFFFFF00) | data & 0xFF;
        resourceManager.setRegister(0, registerA);

        // PC 업데이트
        programCounter += decodedInstruction.getLength();
    }
    
    // COMPR 명령어 실행 
    private void executeCOMPR(DecodedInstruction decodedInstruction) {
        int reg1 = decodedInstruction.getReg1();
        int reg2 = decodedInstruction.getReg2();
  
        int value1 = resourceManager.getRegister(reg1);
        int value2 = resourceManager.getRegister(reg2);
        
        // 레지스터 값 비교 
        if (value1 == value2) {
        	resourceManager.setConditionCode('=');
        } else if (value1 > value2) {
        	resourceManager.setConditionCode('>');
        } else {
        	resourceManager.setConditionCode('<');
        }
        programCounter += decodedInstruction.getLength();
    }
    
    // LDCH 명령어 실행
    private void executeLDCH(DecodedInstruction decodedInstruction) {
    	// LDCH m : A[rightmost byte] <- (m)
    	int TA = calculateTargetAddress(decodedInstruction);
    	resourceManager.setTargetAddress(TA);

    	// X 레지스터 값을 오프셋으로 지정 
        int xValue = resourceManager.getRegister(1); 
        TA += xValue;
    	int value = convertMemoryToInteger(TA, 1);
    	resourceManager.setRegister(0, value);
    	programCounter += decodedInstruction.getLength();
    }
    
    // STCH 명령어 실행
    private void executeSTCH(DecodedInstruction decodedInstruction) {
    	// STCH m : m <- (A) [rightmost byte]
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);
        
        // X 레지스터 값을 오프셋으로 추가
        int xValue = resourceManager.getRegister(1); 
        
        // 레지스터 A의 맨 오른쪽 바이트 추출 
        int value = resourceManager.getRegister(0) & 0x000000FF;

        // TA의 값 변경 
        convertIntegerToMemory(TA + xValue, value, 1);
     
    	// PC 업데이트
        programCounter += decodedInstruction.getLength();
    }
    
    // TIXR 명령어 실행
    private void executeTIXR(DecodedInstruction decodedInstruction) {
    	// TIXR r1 : X <- (X) + 1; (X): (m..m+2)
        resourceManager.setTargetAddress(0);
    	int reg1 = decodedInstruction.getReg1();
    	int value1 = resourceManager.getRegister(reg1);
    	
    	// 레지스터 X의 값 1 증가 
    	int valueX = resourceManager.getRegister(1) + 1;
    	resourceManager.setRegister(1, valueX);
    	
    	// 레지스터 X의 값과 reg1의 값 비교 
    	if (valueX == value1) {
    		resourceManager.setConditionCode('=');
    	} else if (valueX >= value1) {
    		resourceManager.setConditionCode('>');
    	} else {
    		resourceManager.setConditionCode('<');
    	}
    	
    	// PC 업데이트 
    	programCounter += decodedInstruction.getLength();
    }
    
    // JLT 명령어 실행
    private void executeJLT(DecodedInstruction decodedInstruction) {
    	// JLT m : PC <- m if CC set to <
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);
        
        // CC 값 확인
        if (resourceManager.getConditionCode() == '<') {
        	programCounter = TA;
        } else {
        	programCounter += decodedInstruction.getLength();
        }
    }
    
    // STX 명령어 실행
    private void executeSTX(DecodedInstruction decodedInstruction) {
    	// STX m : m..m+2 <- (X)
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);
    	
    	// value 계산
    	int value = resourceManager.getRegister(1);
    	
    	// TA의 값 변경
    	convertIntegerToMemory(TA, value, 3);
    	
    	// PC 업데이트
    	programCounter += decodedInstruction.getLength();
    }
    
    // RSUB 명령어 실행
    private void executeRSUB(DecodedInstruction decodedInstruction) {
    	// RSUB : PC <- (L)
    	int valueL = resourceManager.getRegister(2);
    	
    	programCounter = valueL;
    }
    
    // LDA 명령어 실행 
    private void executeLDA(DecodedInstruction decodedInstruction) {
    	// LDA m : A <- (m .. m+2)
        int TA = 0;
        int value = 0;
        int disp = decodedInstruction.getDisp();

        // Target Address 계산 
        if (!decodedInstruction.ispFlag() && decodedInstruction.isiFlag()) {
            // Direct Addressing
            value = disp;
        } else if (decodedInstruction.ispFlag()) {
            // Simple Addressing
            TA = disp + programCounter + decodedInstruction.getLength();
            value = convertMemoryToInteger(TA, 3);
        }
        resourceManager.setTargetAddress(TA);

        // 레지스터 A의 값 업데이트
        resourceManager.setRegister(0, value);
       
        // PC 업데이트
        programCounter += decodedInstruction.getLength();
    }
    
    // COMP 명령어 실행
    private void executeCOMP(DecodedInstruction decodedInstruction) {
    	// COMP m : (A) : (m..m+2)
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);
    	
    	int valueA = resourceManager.getRegister(0);

    	if (valueA == TA) {
    		resourceManager.setConditionCode('=');
    	} else if (valueA >= TA) {
    		resourceManager.setConditionCode('>');
    	} else {
    		resourceManager.setConditionCode('<');
    	}
    	
    	// PC 업데이트
    	programCounter += decodedInstruction.getLength();
    }
    
    // WD 명령어 실행
    private void executeWD(DecodedInstruction decodedInstruction) {
    	// WD m : Device specified by (m) <- (A)[rightmost byte]
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);
    	
    	// 레지스터 A의 맨 오른쪽 바이트 읽어옴
    	int value = resourceManager.getRegister(0) & 0x000000FF;
    	
    	// TA의 값 변경
    	convertIntegerToMemory(TA, value, 1);
    	
    	// PC 업데이트
    	programCounter += decodedInstruction.getLength();
    }
    
    // J 명령어 실행
    private void executeJ(DecodedInstruction decodedInstruction) {
    	// J m : PC <- m
    	int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);

        // PC 업데이트
        programCounter = TA;
    }

    // STA 명령어 실행
    private void executeSTA(DecodedInstruction decodedInstruction) {
        // STA m : m..m+2 <- (A)
        int TA = calculateTargetAddress(decodedInstruction);
        resourceManager.setTargetAddress(TA);

        // 레지스터 A의 값 가져오기
        int value = resourceManager.getRegister(0);

        // TA의 값 변경
        convertIntegerToMemory(TA, value, 3);

        // PC 업데이트
        programCounter += decodedInstruction.getLength();
    }

    // CLEAR 명령어 실행 
    private void executeCLEAR(DecodedInstruction decodedInstruction) {
        int reg = decodedInstruction.getReg1();
        resourceManager.setRegister(reg, 0);
        
        // PC 업데이트 
        programCounter += decodedInstruction.getLength();
    }

    // 메모리 값을 읽어오는 메소드 
    private int convertMemoryToInteger(int address, int length) {
        int value = 0;
        for (int i = 0; i < length; i++) {
            value = (value << 8) | (resourceManager.getMemory(address + i) & 0xFF);
        }
        return value;
    }
    
    // 메모리 값을 변경하는 메소드
    private void convertIntegerToMemory(int address, int value, int length) {
        for (int i = length - 1; i >= 0; i--) {
            resourceManager.setMemory(address + i, value & 0xFF);
            value >>= 8;
        }
    }
}
