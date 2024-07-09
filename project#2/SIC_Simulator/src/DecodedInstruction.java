public class DecodedInstruction {
	// 오브젝트 코드의 명령어 정보를 관리하는 클래스 
    private int opcode;
    private boolean nFlag;
    private boolean iFlag;
    private boolean xFlag;
    private boolean bFlag;
    private boolean pFlag;
    private boolean eFlag;
    private int disp;
    private int length;
    private int reg1;
    private int reg2;

    // 3/4형식 명령어용 생성자
    public DecodedInstruction(int opcode, boolean nFlag, boolean iFlag, boolean xFlag, boolean bFlag, boolean pFlag, boolean eFlag, int disp, int length) {
        this.opcode = opcode;
        this.nFlag = nFlag;
        this.iFlag = iFlag;
        this.xFlag = xFlag;
        this.bFlag = bFlag;
        this.pFlag = pFlag;
        this.eFlag = eFlag;
        this.disp = disp;
        this.length = length;
    }
    
    // 2형식 명령어용 생성자
    public DecodedInstruction(int opcode, int reg1, int reg2, int length) {
        this.opcode = opcode;
        this.reg1 = reg1;
        this.reg2 = reg2;
        this.length = length;
    }


    public int getOpcode() {
        return opcode;
    }

    public boolean isnFlag() {
        return nFlag;
    }

    public boolean isiFlag() {
        return iFlag;
    }

    public boolean isxFlag() {
        return xFlag;
    }

    public boolean isbFlag() {
        return bFlag;
    }

    public boolean ispFlag() {
        return pFlag;
    }

    public boolean iseFlag() {
        return eFlag;
    }

    public int getDisp() {
        return disp;
    }

    public int getLength() {
        return length;
    }
    
    public int getReg1() {
        return reg1;
    }

    public int getReg2() {
        return reg2;
    }
}
