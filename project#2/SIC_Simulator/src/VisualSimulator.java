import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

public class VisualSimulator {
    private SicXeSimulator simulator;
    private JTextArea logArea;
    private JTextField[] regDecFields;
    private JTextField[] regHexFields;
    private JTextField[] regXEDecFields;
    private JTextField[] regXEHexFields;
    private JTextArea deviceArea; // 장치 상태를 표시할 텍스트 영역
    private JTextField programNameField;
    private JTextField startAddressField;
    private JTextField lengthField;

    public VisualSimulator(SicXeSimulator simulator) {
        this.simulator = simulator;
        this.logArea = new JTextArea(10, 20);
    }

    public void createAndShowGUI() {
        // 메인 프레임
        JFrame frame = new JFrame("SIC/XE Simulator");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(800, 600);
        frame.setLayout(new BorderLayout());

        // 파일 작업을 위한 패널 
        JPanel topPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JLabel fileLabel = new JLabel("FileName:");
        JTextField fileField = new JTextField(20);
        JButton openButton = new JButton("Open");

        topPanel.add(fileLabel);
        topPanel.add(fileField);
        topPanel.add(openButton);

        // 레지스터 값 표시를 위한 패널 
        JPanel leftPanel = new JPanel(new GridLayout(2, 6));
        JPanel regPanel = new JPanel(new GridLayout(5, 2));
        regPanel.setBorder(BorderFactory.createTitledBorder("Register"));

        String[] regNames = {"A (#0)", "X (#1)", "L (#2)", "PC (#8)", "SW (#9)"};
        regDecFields = new JTextField[regNames.length];
        regHexFields = new JTextField[regNames.length];

        for (int i = 0; i < regNames.length; i++) {
            regPanel.add(new JLabel(regNames[i]));
            regDecFields[i] = new JTextField();
            regPanel.add(regDecFields[i]);
            regHexFields[i] = new JTextField();
            regPanel.add(regHexFields[i]);
        }

        JPanel regXEPanel = new JPanel(new GridLayout(4, 2));
        regXEPanel.setBorder(BorderFactory.createTitledBorder("Register (for XE)"));

        String[] regXENames = {"B (#3)", "S (#4)", "T (#5)", "F (#6)"};
        regXEDecFields = new JTextField[regXENames.length];
        regXEHexFields = new JTextField[regXENames.length];

        for (int i = 0; i < regXENames.length; i++) {
            regXEPanel.add(new JLabel(regXENames[i]));
            regXEDecFields[i] = new JTextField();
            regXEPanel.add(regXEDecFields[i]);
            regXEHexFields[i] = new JTextField();
            regXEPanel.add(regXEHexFields[i]);
        }

        leftPanel.add(regPanel);
        leftPanel.add(regXEPanel);
        
        // "사용중인 장치" 패널 추가
        JPanel devicePanel = new JPanel(new BorderLayout());
        devicePanel.setBorder(BorderFactory.createTitledBorder("사용중인 장치"));
        deviceArea = new JTextArea();
        deviceArea.setLineWrap(true);
        JScrollPane deviceScrollPane = new JScrollPane(deviceArea);
        devicePanel.add(deviceScrollPane, BorderLayout.CENTER);
        leftPanel.add(devicePanel);

        // Create panels for the center section (H and E records)
        JPanel centerPanel = new JPanel();
        centerPanel.setLayout(new BoxLayout(centerPanel, BoxLayout.Y_AXIS));

        JPanel headerRecordPanel = new JPanel(new GridLayout(3, 2));
        headerRecordPanel.setBorder(BorderFactory.createTitledBorder("H (Header Record)"));

        headerRecordPanel.add(new JLabel("Program Name:"));
        programNameField = new JTextField();
        headerRecordPanel.add(programNameField);

        headerRecordPanel.add(new JLabel("Start Address of Object Program:"));
        startAddressField = new JTextField();
        headerRecordPanel.add(startAddressField);

        headerRecordPanel.add(new JLabel("Length of Program:"));
        lengthField = new JTextField();
        headerRecordPanel.add(lengthField);

        JPanel endRecordPanel = new JPanel(new GridLayout(1, 2));
        endRecordPanel.setBorder(BorderFactory.createTitledBorder("E (End Record)"));

        endRecordPanel.add(new JLabel("Address of First Instruction in Object Program:"));
        JTextField firstInstructionField = new JTextField();
        endRecordPanel.add(firstInstructionField);

        JPanel instructionPanel = new JPanel(new BorderLayout());
        instructionPanel.setBorder(BorderFactory.createTitledBorder("Instructions"));

        JTextArea instructionArea = new JTextArea();
        instructionArea.setLineWrap(true);
        JScrollPane scrollPane = new JScrollPane(instructionArea);

        JPanel addressPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JLabel targetAddressLabel = new JLabel("Target Address:");
        JTextField targetAddressField = new JTextField(20);
        addressPanel.add(targetAddressLabel);
        addressPanel.add(targetAddressField);

        JButton stepButton = new JButton("실행 (1 Step)");
        JButton runAllButton = new JButton("실행 (All)");
        JButton stopButton = new JButton("종료");

        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        buttonPanel.add(stepButton);
        buttonPanel.add(runAllButton);
        buttonPanel.add(stopButton);

        instructionPanel.add(addressPanel, BorderLayout.NORTH);
        instructionPanel.add(scrollPane, BorderLayout.CENTER);
        instructionPanel.add(buttonPanel, BorderLayout.SOUTH);

        centerPanel.add(headerRecordPanel);
        centerPanel.add(endRecordPanel);
        centerPanel.add(instructionPanel);

        // Create a panel for the bottom section (Log)
        JPanel logPanel = new JPanel(new BorderLayout());
        logPanel.setBorder(BorderFactory.createTitledBorder("Log (명령어 수행 관련)"));
        logArea.setLineWrap(true);
        JScrollPane logScrollPane = new JScrollPane(logArea);
        logPanel.add(logScrollPane, BorderLayout.CENTER);

        // Add all panels to the main frame
        frame.add(topPanel, BorderLayout.NORTH);
        frame.add(leftPanel, BorderLayout.WEST);
        frame.add(centerPanel, BorderLayout.CENTER);
        frame.add(logPanel, BorderLayout.SOUTH);

        // Add action listener for the open button
        openButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                int result = fileChooser.showOpenDialog(frame);
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    fileField.setText(selectedFile.getAbsolutePath());
                    try {
                        simulator.loadObjectCode(selectedFile.getAbsolutePath());
                        logArea.append("Object code loaded successfully.\n");
                        displayLoaderInfo();
                        firstInstructionField.setText(String.format("%06X", simulator.getProgStartAddress()));
                    } catch (Exception ex) {
                        logArea.append("Error loading object code: " + ex.getMessage() + "\n");
                    }
                }
            }
        });

        // Add action listener for the step button
        stepButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    if (simulator.hasNextInstruction()) {
                        simulator.executeNextInstruction(); // 수정된 부분
                        instructionArea.append(simulator.getLastExecutedObjectCode() + "\n");
                        logArea.append(simulator.getCurrentInstructionName() + "\n"); 
                        updateRegisterValues(); // 레지스터 값 업데이트
                        updateDeviceStatus(); // 장치 상태 업데이트 
                        displayLoaderInfo();
                        targetAddressField.setText(String.format("%06X", simulator.getTargetAddress()));
                        firstInstructionField.setText(String.format("%06X", simulator.getProgStartAddress()));
                    } else {
                        logArea.append("No more instructions to execute.\n");
                    }
                } catch (RuntimeException ex) {
                    logArea.append("Program terminated: " + ex.getMessage() + "\n");
                    System.out.println("Program terminated: " + ex.getMessage());
                } catch (Exception ex) {
                    logArea.append("Error during step execution: " + ex.getMessage() + "\n");
                }
            }
        });

        // Add action listener for the run all button
        runAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    while (simulator.hasNextInstruction()) {
                        simulator.executeNextInstruction(); // 수정된 부분
                        instructionArea.append(simulator.getLastExecutedObjectCode() + "\n");
                        logArea.append(simulator.getCurrentInstructionName() + "\n"); 
                        updateRegisterValues(); // 레지스터 값 업데이트
                        updateDeviceStatus(); // 장치 상태 업데이트 
                        displayLoaderInfo();
                        targetAddressField.setText(String.format("%06X", simulator.getTargetAddress()));
                        firstInstructionField.setText(String.format("%06X", simulator.getProgStartAddress()));
                    }
                } catch (Exception ex) {
                    logArea.append("Error during execution: " + ex.getMessage() + "\n");
                }
            }
        });

        // Add action listener for the stop button
        stopButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                simulator.stopExecution();
                logArea.append("Execution stopped.\n");
            }
        });

        frame.setVisible(true);
    }

    // 레지스터 값 업데이트 메서드
    private void updateRegisterValues() {
        int[] registerIndices = {0, 1, 2, 8, 9}; // 일반 레지스터 인덱스
        int[] registerXEIndices = {3, 4, 5, 6}; // XE 레지스터 인덱스

        // 일반 레지스터 값 업데이트
        for (int i = 0; i < registerIndices.length; i++) {
            int value = simulator.getRegister(registerIndices[i]);
            if (registerIndices[i] == 8) {
                // 레지스터 8번은 프로그램 카운터(PC)
                value = simulator.getProgramCounter();
            }
            regDecFields[i].setText(String.valueOf(value));
            regHexFields[i].setText(String.format("%06X", value));        
        }

        // XE 레지스터 값 업데이트
        for (int i = 0; i < registerXEIndices.length; i++) {
            int value = simulator.getRegister(registerXEIndices[i]);
            regXEDecFields[i].setText(String.valueOf(value));
            regXEHexFields[i].setText(String.format("%06X", value));
        }
    }

    // 장치 상태 업데이트 메서드
    private void updateDeviceStatus() {
        Integer activeDevice = simulator.getActiveDevice();
        if (activeDevice != null) {
            deviceArea.setText(String.format("Device %02X is active\n", activeDevice));
        } else {
            deviceArea.setText("No active device\n");
        }
    }

    private void displayLoaderInfo() {
        programNameField.setText(simulator.getProgName());
        startAddressField.setText(String.format("%06X", simulator.getProgStartAddress())); 
        lengthField.setText(String.format("%06X", simulator.getProgLength()));
    }
}
