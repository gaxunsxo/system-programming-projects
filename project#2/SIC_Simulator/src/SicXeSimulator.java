import java.io.IOException;

public class SicXeSimulator {
	private ResourceManager resourceManager;
    private ObjectCodeLoader loader;
    private InstructionExecutor executor;
    private boolean running;

    public SicXeSimulator() {
        this.resourceManager = new ResourceManager();
        this.loader = new ObjectCodeLoader(resourceManager);
        this.running = true;
        try {
            this.executor = new InstructionExecutor(resourceManager);
        } catch (IllegalArgumentException e) {
            System.err.println("Error initializing InstructionExecutor: " + e.getMessage());
        }
    }

    // 오브젝트 코드 로드 요청
    public void loadObjectCode(String filePath) throws IOException {
        loader.load(filePath);
        // 로딩 후 프로그램 카운터를 초기화
        executor = new InstructionExecutor(resourceManager);
    }
    
    // 다음 명령어가 존재하는지 확인 
    public boolean hasNextInstruction() {
        return running && executor.hasNextInstruction();
    }
    
    // 다음 명령어 실행 
    public void executeNextInstruction() {
        try {
            executor.executeNextInstruction();
        } catch (RuntimeException e) {
            System.out.println("Program terminated: " + e.getMessage());
            running = false;
        }
    }
    
    public void stopExecution() {
        running = false;
    }
    
    // 마지막으로 실행된 오브젝트 코드 반환
    public String getLastExecutedObjectCode() {
        return executor.getLastObjectCode();
    }

    // 현재 명령어의 이름을 반환
    public String getCurrentInstructionName() {
        return executor.getCurrentInstructionName();
    }
    
    
    public int getRegister(int index) {
        return resourceManager.getRegister(index);
    }
    
   
    public Integer getActiveDevice() {
        return resourceManager.getActiveDevice();
    }

    // 전체 프로그램의 정보를 반환하는 메서드 
    public int getProgramCounter() {
        return executor.getProgramCounter();
    }

    public String getProgName() {
        return resourceManager.getTotalProgName();
    }

    public int getProgLength() {
        return resourceManager.getTotalProgLength();
    }

    public int getProgStartAddress() {
        return resourceManager.getTotalProgStartAddress();
    }

    public int getProgFirstInstructionAddress() {
        return resourceManager.getFirstInstructionAddress();
    }
    
    // 타겟 주소를 반환하는 메서드 
    public int getTargetAddress() {
        return resourceManager.getTargetAddress();
    }
}
