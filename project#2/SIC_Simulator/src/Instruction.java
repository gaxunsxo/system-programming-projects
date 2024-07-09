import java.util.HashMap;
import java.util.Map;

public class Instruction {
    private String name;
    private int opcode;
    private int format;

    public Instruction(String name, int opcode, int format) {
        this.name = name;
        this.opcode = opcode;
        this.format = format;
    }

    public String getName() {
        return name;
    }

    public int getOpcode() {
        return opcode;
    }

    public int getFormat() {
        return format;
    }

    public static Instruction getInstruction(int opcode) {
        return instructionSet.get(opcode);
    }

    private static final Map<Integer, Instruction> instructionSet = new HashMap<>();

    static {
        instructionSet.put(0xB4, new Instruction("CLEAR", 0xB4, 2));
        instructionSet.put(0x28, new Instruction("COMP", 0x28, 3));
        instructionSet.put(0xA0, new Instruction("COMPR", 0xA0, 2));
        instructionSet.put(0x3C, new Instruction("J", 0x3C, 3));
        instructionSet.put(0x30, new Instruction("JEQ", 0x30, 3));
        instructionSet.put(0x38, new Instruction("JLT", 0x38, 3));
        instructionSet.put(0x48, new Instruction("JSUB", 0x48, 3));
        instructionSet.put(0x00, new Instruction("LDA", 0x00, 3));
        instructionSet.put(0x50, new Instruction("LDCH", 0x50, 3));
        instructionSet.put(0x74, new Instruction("LDT", 0x74, 3));
        instructionSet.put(0xD8, new Instruction("RD", 0xD8, 3));
        instructionSet.put(0x4C, new Instruction("RSUB", 0x4C, 3));
        instructionSet.put(0x0C, new Instruction("STA", 0x0C, 3));
        instructionSet.put(0x14, new Instruction("STL", 0x14, 3));
        instructionSet.put(0x54, new Instruction("STCH", 0x54, 3));
        instructionSet.put(0x10, new Instruction("STX", 0x10, 3));
        instructionSet.put(0x1C, new Instruction("SUB", 0x1C, 3));
        instructionSet.put(0xE0, new Instruction("TD", 0xE0, 3));
        instructionSet.put(0xB8, new Instruction("TIXR", 0xB8, 2));
        instructionSet.put(0xDC, new Instruction("WD", 0xDC, 3));
    }
}
