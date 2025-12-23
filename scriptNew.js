  
    // State management
    import createModule from "./riscv_web.js";
    let Module, sim;
    createModule().then((mod) => {
        Module = mod;

        // Wait for runtime to finish initializing
        if (Module.calledRun) {
            initializeSimulator();
        } else {
            Module.onRuntimeInitialized = initializeSimulator;
        }

    function initializeSimulator() {
        console.log("✅ Runtime initialized");
        sim = new Module.Simulator();
        setupSimulator();
        // Now FS is also available
        if (!Module.FS) {
        console.error("❌ Module.FS is undefined even after initialization.");
        }
    }
    });

    let breakpoints = new Set();
    let memoryData = new Map();
    let cacheStats = { hits: 0, misses: 0 };
    let currentMemoryBase = 0x10010000;
    let currentLine = -1;
    let isRunning = false;

    // Sample RISC-V code
    const sampleCode = `main:
    addi x4, x0, 4
    add x19, x0, x4
    addi x14, x0, 2
    sub x19, x19, x14`;

  

    function setupSimulator() {
            
      // Load sample code on startup
      updateAllViews();
      setupUIEventListeners();
    }

    // UI Elements
    const consoleOutput = document.getElementById("consoleOutput");
    const codeTextarea = document.getElementById("codeTextarea");
    const machineCodeTextarea = document.getElementById("machineCodeTextarea");
    const lineNumbers = document.getElementById("lineNumbers");
    const registerFormatToggle = document.getElementById("registerFormat");

    function logToConsole(msg) {
      const timestamp = new Date().toLocaleTimeString();
      consoleOutput.textContent += `[${timestamp}] ${msg}\n`;
      consoleOutput.scrollTop = consoleOutput.scrollHeight;
    }

    function clearConsole() {
      consoleOutput.textContent = "";
    }

    function checkReady() {
      if (!Module || !sim || !Module.FS || typeof Module.FS.writeFile !== "function") {
        if (!Module) logToConsole("❌ Module not initialized. Please wait...");
        if(!sim) logToConsole("❌ Simulator not initialized. Please wait...");
        if(!Module.FS) logToConsole("❌ File System not available. Please wait...");
        if(typeof Module.FS.writeFile !== "function") logToConsole("❌ FS.writeFile not available. Please wait...");
        logToConsole("❌ Simulator not ready. Try again in a few seconds.");
        return false;
      }
      return true;
    }

    

    // --- NEW UI EVENT LISTENERS ---
    function setupUIEventListeners() {
        // File Menu
        const loadProgramBtn = document.getElementById('loadProgramBtn');

        loadProgramBtn.addEventListener('click', (e) => {
            e.preventDefault();
            loadManualCode(); // Reuse the full loader with sanitation and FS write
            closeMenu(); 
        });

        const loadSample = document.getElementById('loadSample');

        loadSample.addEventListener('click', (e) => {
            e.preventDefault();
            loadSampleCode(); 
            closeMenu(); 
        });

        //register format toggle
        setupRegisterFormatToggle();
        const saveAsBtn = document.getElementById('saveAsBtn');
        // Save As button         
        saveAsBtn.addEventListener('click', async (e) => {
            e.preventDefault();
            const code = codeTextarea.value;

            try {
                const handle = await window.showSaveFilePicker({
                    suggestedName: 'riscv_program.s',
                    types: [
                        {
                            description: 'RISC-V Assembly',
                            accept: { 'text/plain': ['.s','.asm'] }
                        }
                    ]
                });

                const writable = await handle.createWritable();
                await writable.write(code);
                await writable.close();

                logToConsole('💾 Program saved using native file picker!');
            } catch (err) {
                logToConsole('❌ Save cancelled or failed: ' + err.message);
            }
        });
        
        

        // Run Menu 
        document.getElementById('runMenuBtn').addEventListener('click', (e) => {
            e.preventDefault();
            runSimulator();
        });
        document.getElementById('stepMenuBtn').addEventListener('click', (e) => {
            e.preventDefault();
            stepSimulator();
        });

        // Side Navigation Panel
        const sideNav = document.getElementById('sideNav');
        const rightPanePanels = {
            register: document.getElementById('registerView'),
            cache: document.getElementById('cacheView'),
            memory: document.getElementById('memoryView'),
            stack: document.getElementById('stackView')
        };
        
        sideNav.addEventListener('click', (e) => {
            const button = e.target.closest('.side-nav-btn');
            if (!button) return;
            
            document.querySelectorAll('.side-nav-btn').forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
            
            const viewToShow = button.dataset.view;
            
            // Hide all panels
            Object.values(rightPanePanels).forEach(panel => {
                if(panel) panel.style.display = 'none';
            });
            
            // Show the selected panel(s)
            if (viewToShow === 'register') {
                if(rightPanePanels.register) rightPanePanels.register.style.display = 'flex';
                if(rightPanePanels.stack) rightPanePanels.stack.style.display = 'flex';
            } else if (rightPanePanels[viewToShow]) {
                rightPanePanels[viewToShow].style.display = 'flex';
            }
        });
    }

    // Close dropdown menu
    function closeMenu() {
    const openMenus = document.querySelectorAll('.dropdown-content.show');
    openMenus.forEach(menu => menu.classList.remove('show'));
}
    // Breakpoint management
    function toggleBreakpoint(line) {
      if (breakpoints.has(line)) {
        breakpoints.delete(line);
        logToConsole(`🔴 Removed breakpoint at line ${line}`);
      } else {
        breakpoints.add(line);
        logToConsole(`🔴 Added breakpoint at line ${line}`);
      }
      updateLineNumbers();
      updateHighlightOverlay();
    }

    function updateLineNumbers() {
      const lines = codeTextarea.value.split('\n');
      lineNumbers.innerHTML = '';
      
      lines.forEach((_, index) => {
        const lineNum = index + 1;
        const lineDiv = document.createElement('div');
        lineDiv.className = 'line-number';
        lineDiv.textContent = lineNum;
        
        if (breakpoints.has(lineNum)) {
          lineDiv.classList.add('breakpoint');
        }
        
        lineDiv.onclick = () => toggleBreakpoint(lineNum);
        lineNumbers.appendChild(lineDiv);
      });
    }

    function updateHighlightOverlay() {
      
      breakpoints.forEach(line => {
        const bpHighlight = document.createElement("div");
        bpHighlight.className = "breakpointLine";
        bpHighlight.style.top = `${(line - 1) * 18}px`;
        bpHighlight.style.height = "18px";
        overlay.appendChild(bpHighlight);
      });
    }

    // Code editing and compilation
    function updateMachineCode() {
      if (!checkReady()) return;
      
      try {
          machineCodeTextarea.value = sim.printFormattedAssembly();
        } catch (error) {
        logToConsole("❌ Error generating machine code: " + error.message);
      }
    }


    // Get the current format preference
    function isHexFormat() {
        return !document.getElementById("registerFormat").checked; // unchecked = hex, checked = decimal
    }
  
    function formatRegisterValue(hexValue) {
    if (!hexValue || hexValue === "0" || hexValue === "00000000") {
        return isHexFormat() ? "00000000" : "0";
    }
    
     // Always parse from hex since register values come in as hex
    const numValue = parseInt(hexValue, 16);
    
    if (isHexFormat()) {
        // Return hex value, ensure it's padded to 8 characters
        return hexValue.padStart(8, '0').toUpperCase();
    } else {
        // Return decimal value
        // Handle signed interpretation for 32-bit values
        const signed32bit = numValue > 0x7FFFFFFF ? numValue - 0x100000000 : numValue;
        return signed32bit.toString();
    }
    }

    // Register table management
    // Store previous register values to detect changes
    const previousRegisterValues = {};
    function updateRegisterTable() {
    if (!checkReady()) return;

    try {
        const registers = sim.getRegisters().split(/[\n\t]+/).filter(line => line.trim());
        const registerTableBody = document.querySelector("#registerTable tbody");
        registerTableBody.innerHTML = "";

        const registerAliases = {
            x0: "zero", x1: "ra", x2: "sp", x3: "gp", x4: "tp", x5: "t0", x6: "t1", x7: "t2",
            x8: "s0", x9: "s1", x10: "a0", x11: "a1", x12: "a2", x13: "a3", x14: "a4", x15: "a5",
            x16: "a6", x17: "a7", x18: "s2", x19: "s3", x20: "s4", x21: "s5", x22: "s6", x23: "s7",
            x24: "s8", x25: "s9", x26: "s10", x27: "s11", x28: "t3", x29: "t4", x30: "t5", x31: "t6"
        };

        const pairs = registers.map(line => {
            const [reg, val] = line.trim().split(":");
            return [reg.trim(), val.trim()];
        });

        for (let i = 0; i < 16; i++) {
            const row = document.createElement("tr");

            for (let col = 0; col < 2; col++) {
                const idx = i + col * 16;
                if (idx < pairs.length) {
                    const [reg, hexVal] = pairs[idx];
                    const alias = registerAliases[reg] || "";
                    
                    // Format the value based on current setting
                    const formattedVal = formatRegisterValue(hexVal);
                    
                    // Check for changes (compare original hex values)
                    const prevVal = previousRegisterValues[reg];
                    const changed = prevVal !== undefined && prevVal !== hexVal;
                    previousRegisterValues[reg] = hexVal; // Store hex value for comparison

                    const nameTd = document.createElement("td");
                    nameTd.textContent = reg;

                    const aliasTd = document.createElement("td");
                    aliasTd.textContent = alias;

                    const valTd = document.createElement("td");
                    valTd.textContent = formattedVal;
                    
                    // Check if zero (works for both hex and decimal)
                    const isZero = hexVal === "0" || hexVal === "00000000" || parseInt(hexVal, 16) === 0;
                    valTd.className = isZero ? "register-zero" : "register-nonzero";
                    valTd.style.fontFamily = "monospace";

                    if (changed) {
                        valTd.classList.add("highlight-change");
                        valTd.addEventListener("animationend", () => {
                            valTd.classList.remove("highlight-change");
                        }, { once: true });
                    }

                    row.appendChild(nameTd);
                    row.appendChild(aliasTd);
                    row.appendChild(valTd);
                } else {
                    // Fill in blank cells to maintain structure
                    row.innerHTML += "<td></td><td></td><td></td>";
                }
            }

            registerTableBody.appendChild(row);
        }

    } catch (err) {
        logToConsole("❌ Error updating register table: " + err.message);
    }
}

// Setup event listener for register format toggle
function setupRegisterFormatToggle() {
    const toggle = document.getElementById("registerFormat");
    if (toggle) {
        toggle.addEventListener("change", function() {
            // Refresh the register table when format changes
            updateRegisterTable();
        });
    }
}



   function updateStackView() {
  if (!checkReady()) return;

  try {
    const stackText = sim.getStack();  // your current stack as multiline string
    
    // Example input (simulate):
    // Call stack (oldest to newest):
    // #0 main at line 7 Next Instruction 0x00000020
    // #1 factorial at line 34 Next Instruction 0x00000020
    // #2 factorial at line 34 Next Instruction 0x00000020
    // ...
    
    const lines = stackText.split('\n');
    
    // Create a container element (e.g., a <ul>) for the tree view
    const ul = document.createElement('ul');
    
    // Start from line 1 or line 2 to skip the header line "Call stack..."
    for (let i = 1; i < lines.length; i++) {
      const line = lines[i].trim();
      if (line.length === 0) continue;
      
      // Create a list item for each frame
      const li = document.createElement('li');
      li.textContent = line;
      ul.appendChild(li);
    }
    
    // Clear old content and add new tree
    stackView.innerHTML = ''; 
    stackView.appendChild(ul);

  } catch (err) {
    logToConsole("❌ Error updating stack view: " + err.message);
  }
}
 
    
        function loadCodeFromString(content, sourceName = "Code") {
        // 🧽 Normalize and sanitize
            content = content.replace(/\r\n/g, "\n");
            if (content.charCodeAt(0) === 0xFEFF) {
                content = content.slice(1); // Remove BOM
            }

            // ✅ Populate textarea
            const textarea = document.getElementById("codeTextarea");
            textarea.value = content;

            // ✅ Virtual FS setup
            if (!Module.FS.analyzePath("/tmp").exists) {
                Module.FS.mkdir("/tmp");
            }

            const filename = "/tmp/manual.s"; // Optionally make this dynamic
            Module.FS.writeFile(filename, content);

            // ✅ Call sim.load and update UI
            let lineNumber = sim.load(content);
            highlightLine(parseInt(lineNumber));
            updateLineNumbers();
            updateMachineCode();
            currentLine = parseInt(lineNumber);

            logToConsole(`✅ Loaded ${sourceName}`);
        }

    function loadSampleCode() {
        loadCodeFromString(sampleCode, "Sample code");
    }

    function runSimulator() {
      if (!checkReady()) return;
      try {
        const output =sim.run();
        logToConsole(output);     // Print it to the console
        updateRegisterTable();
        updateStackView();
      } catch (err) {
        logToConsole("❌ Error during run: " + err.message);
      }
    }


      function highlightLine(lineNum) {
        const textarea = document.getElementById("codeTextarea");
        const overlay = document.getElementById("highlightOverlay");
        const machineOverlay = document.getElementById("machineHighlightOverlay");
        const machineCodeTextArea = document.getElementById("machineCodeTextarea");
        if (!textarea || !overlay  || !machineOverlay || !machineCodeTextArea) return;

        const lineHeight = 18; // match line-height in CSS
        const scrollTop = textarea.scrollTop;
        const paddingTop = parseFloat(getComputedStyle(overlay).paddingTop) || 0;

        // === Assembly view highlight ===
        let highlighter = overlay.querySelector(".highlightLine");
        if (!highlighter) {
          highlighter = document.createElement("div");
          highlighter.className = "highlightLine";
          overlay.appendChild(highlighter);
        }

        const topPos = (lineNum - 1) * lineHeight - scrollTop + paddingTop;
        highlighter.style.top = `${topPos}px`;
        highlighter.style.height = `${lineHeight}px`;

        // === Machine code view highlight ===
        machineOverlay.innerHTML = ''; // Clear previous highlight

        const machineLine = sim.getNextBinLine()+1; // get line from sim for this assembly line
        
        const machinePaddingTop = parseFloat(getComputedStyle(machineCodeTextarea).paddingTop) || 0;
        const machineScrollTop = machineCodeTextarea.scrollTop;
        

  if (machineLine !== undefined && !isNaN(machineLine)) {
        const mcHighlighter = document.createElement("div");
        mcHighlighter.className = "highlightLine";
        // Calculate position relative to the machine code textarea's scroll and padding
        const mcTopPos = (machineLine - 1) * lineHeight - machineScrollTop + machinePaddingTop;
        mcHighlighter.style.top = `${mcTopPos}px`;
        mcHighlighter.style.height = `${lineHeight}px`;
        machineOverlay.appendChild(mcHighlighter);

        // Optional: scroll machine code view if needed
        const mcVisibleTop = machineScrollTop;
        const mcVisibleBottom = machineScrollTop + machineCodeTextarea.clientHeight;
        const mcTargetTopForScroll = (machineLine - 1) * lineHeight;

        if (mcTargetTopForScroll < mcVisibleTop || mcTargetTopForScroll + lineHeight > mcVisibleBottom) {
            machineCodeTextarea.scrollTop = mcTargetTopForScroll - lineHeight * 3;
        }
    }

        // Auto-scroll if needed
        const visibleTop = textarea.scrollTop;
        const visibleBottom = textarea.scrollTop + textarea.clientHeight;
        const targetTop = (lineNum - 1) * lineHeight;

        if (targetTop < visibleTop || targetTop + lineHeight > visibleBottom) {
          textarea.scrollTop = targetTop - lineHeight * 3; // some padding
        }
      }



    //document.getElementById("codeTextarea").addEventListener("scroll", () => {
    //document.getElementById("highlightOverlay").scrollTop = document.getElementById("codeTextarea").scrollTop;
    //});


    // Add scroll event listener when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    const codeTextarea = document.getElementById("codeTextarea");
    const highlightOverlay = document.getElementById("highlightOverlay");
    const machineCodeTextarea = document.getElementById("machineCodeTextarea");
    const machineHighlightOverlay = document.getElementById("machineHighlightOverlay");
    
    // Assembly code scroll handler
    if (codeTextarea && highlightOverlay) {
        codeTextarea.addEventListener("scroll", () => {
                        // Re-calculate highlight position when scrolling to keep it fixed
            const currentHighlight = highlightOverlay.querySelector(".highlightLine");
            if (currentHighlight && sim) {
                const assemblyLine = sim.getCurrentLine() + 1; // or however you get the current line
                if (assemblyLine !== undefined && !isNaN(assemblyLine)) {
                    const lineHeight = 18;
                    const assemblyPaddingTop = parseFloat(getComputedStyle(highlightOverlay).paddingTop) || 0;
                    const assemblyScrollTop = codeTextarea.scrollTop;
                    const asmTopPos = (assemblyLine - 1) * lineHeight - assemblyScrollTop + assemblyPaddingTop;
                    currentHighlight.style.top = `${asmTopPos}px`;
                }
            }
        });
    }
    
    // Add scroll sync for machine code view
    if (machineCodeTextarea && machineHighlightOverlay) {
        machineCodeTextarea.addEventListener("scroll", () => {
            // Re-calculate highlight position when scrolling
            const currentHighlight = machineHighlightOverlay.querySelector(".highlightLine");
            if (currentHighlight && sim) {
                const machineLine = sim.getNextBinLine() + 1;
                if (machineLine !== undefined && !isNaN(machineLine)) {
                    const lineHeight = 18;
                    const machinePaddingTop = parseFloat(getComputedStyle(machineCodeTextarea).paddingTop) || 0;
                    const machineScrollTop = machineCodeTextarea.scrollTop;
                    const mcTopPos = (machineLine - 1) * lineHeight - machineScrollTop + machinePaddingTop;
                    currentHighlight.style.top = `${mcTopPos}px`;
                }
            }
        });
    }
});

    function stepSimulator() {
      if (!checkReady()) return;
      try {
        let lineNumber = parseInt(sim.step()) ; // returns 1-based line number
        
        if (lineNumber === -1 || lineNumber === 0) {
          logToConsole("✅ Simulation completed or no more steps available.");
          // Clear highlight overlay
          const overlay = document.getElementById("highlightOverlay");
          const highlighter = overlay.querySelector(".highlightLine");
          if (highlighter) {
            highlighter.remove();
          }
          

          return;
        }
        //logToConsole(`➡️ Line ${lineNumber}`);
        
        highlightLine(parseInt(lineNumber));
        updateRegisterTable();
        updateStackView();
      } catch (err) {
        logToConsole("❌ Error during step: " + err.message);
      }
    }

function loadManualCode() {
  if (!checkReady()) return;

  const input = document.createElement("input");
  input.type = "file";
  input.accept = ".s,.asm"; // Allow only .s and .asm files
  input.style.display = "none";

  input.onchange = async (event) => {
    const file = event.target.files[0];
    if (!file) {
      logToConsole("⚠️ No file selected.");
      return;
    }

    try {
      const content = await file.text();
      loadCodeFromString(content, file.name);
    } catch (e) {
      logToConsole("❌ Error loading file: " + e.message);
    }
    };

  document.body.appendChild(input);
  input.click();
  document.body.removeChild(input);
}

document.addEventListener("keydown", (e) => {
  if (e.key === "F10") {
    e.preventDefault(); //  prevents browser focus-stealing
    stepSimulator();
  }
  if (e.key === "F9") {
    e.preventDefault(); // Also avoid accidental page reload
    runSimulator();
  }
});

    window.clearConsole = clearConsole;

    function updateAllViews() {
        updateLineNumbers();
        updateMachineCode();
        updateRegisterTable();
    }

   
  