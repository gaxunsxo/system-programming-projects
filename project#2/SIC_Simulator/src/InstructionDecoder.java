public class InstructionDecoder {
	// 오브젝트 코드를 디코딩하여 'DecodedInstruction' 객체를 생성하는 클래스 
    public static DecodedInstruction decode(int programCounter, ResourceManager resourceManager) {
        int firstByte = resourceManager.getMemory(programCounter);
        
        int opcode = firstByte & 0xFC; // 상위 6비트를 추출하여 opcode를 얻음
        Instruction instruction = Instruction.getInstruction(opcode);

        
        int format = instruction.getFormat();

        if (format == 2) {
            // 2형식 명령어 처리
            int reg1 = (resourceManager.getMemory(programCounter + 1) & 0xF0) >> 4;
            int reg2 = resourceManager.getMemory(programCounter + 1) & 0x0F;
            return new DecodedInstruction(opcode, reg1, reg2, 2);
        }

        
        String first3digits = String.format("%02X%02X%02X",
                resourceManager.getMemory(programCounter),
                resourceManager.getMemory(programCounter + 1),
                resourceManager.getMemory(programCounter + 2));
        int nixbpe = Integer.parseInt(first3digits.substring(1, 3), 16) & 0x3F;

        boolean nFlag = (nixbpe & 0x20) != 0;
        boolean iFlag = (nixbpe & 0x10) != 0;
        boolean xFlag = (nixbpe & 0x08) != 0;
        boolean bFlag = (nixbpe & 0x04) != 0;
        boolean pFlag = (nixbpe & 0x02) != 0;
        boolean eFlag = (nixbpe & 0x01) != 0;

        int disp;
        int length = 3;

        if (eFlag) {
        	// 4형식 명령어: 하위 5자리 추출 
        	disp = ((resourceManager.getMemory(programCounter + 1) & 0x0F) << 16) |
                     (resourceManager.getMemory(programCounter + 2) << 8) |
                     resourceManager.getMemory(programCounter + 3);
            length = 4;
            
            // 20비트 값을 부호 있는 값으로 변환
            if ((disp & 0x80000) != 0) {
                disp |= 0xFFF00000; // 부호 확장
            }
        } else {
        	// 3형식 명령어: 하위 3자리 추출 
        	disp = ((resourceManager.getMemory(programCounter + 1) & 0x0F) << 8) |
                    resourceManager.getMemory(programCounter + 2);
        	
        	// 12비트 값을 부호 있는 값으로 변환
            if ((disp & 0x800) != 0) {
                disp |= 0xFFFFF000; // 부호 확장
            }
        }

        return new DecodedInstruction(opcode, nFlag, iFlag, xFlag, bFlag, pFlag, eFlag, disp, length);
    }
}
