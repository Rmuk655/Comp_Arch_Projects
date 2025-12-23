import createModule from "./riscv_web.js";
let Module;
let sim;

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

    // Now FS is also available
    if (!Module.FS) {
          console.error("❌ Module.FS is undefined even after initialization.");
    }
  }
});


    const consoleOutput = document.getElementById("consoleOutput");
    const codeTextarea = document.getElementById("codeTextarea");
    const registerTableBody = document.querySelector("#registerTable tbody");
    const stackView = document.getElementById("stackView");

    function logToConsole(msg) {
      consoleOutput.textContent += msg + '\n';
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

// Keep previous register values globally (outside the function)
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

    for (let i = 0; i < pairs.length; i += 2) {
      const row = document.createElement("tr");

      for (let j = 0; j < 2; j++) {
        if (i + j < pairs.length) {
          const [reg, val] = pairs[i + j];
          const alias = registerAliases[reg] || "";
          const valClass = (val === "0" || val === "00000000") ? "register-zero" : "register-nonzero";

          // Check if value changed compared to previous
          const prevVal = previousRegisterValues[reg];
          const changed = prevVal !== undefined && prevVal !== val;

          // Save new value for next update
          previousRegisterValues[reg] = val;

          // Build the <td> for value with possible highlight class
          const valueTd = document.createElement("td");
          valueTd.textContent = val;
          valueTd.className = valClass;
          if (changed) {
            valueTd.classList.add("highlight-change");
            // Remove the animation class after animation ends
            valueTd.addEventListener("animationend", () => {
              valueTd.classList.remove("highlight-change");
            }, { once: true });
          }

          row.innerHTML += `<td>${reg}</td><td>${alias}</td>`;
          row.appendChild(valueTd);
        } else {
          row.innerHTML += "<td></td><td></td><td></td>";
        }
      }

      registerTableBody.appendChild(row);
    }
  } catch (err) {
    logToConsole("❌ Error updating register table: " + err.message);
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
        if (!textarea || !overlay) return;

        const lineHeight = 20; // match line-height in CSS
        const scrollTop = textarea.scrollTop;
        const paddingTop = parseFloat(getComputedStyle(textarea).paddingTop) || 0;

        let highlighter = overlay.querySelector(".highlightLine");
        if (!highlighter) {
          highlighter = document.createElement("div");
          highlighter.className = "highlightLine";
          overlay.appendChild(highlighter);
        }

        const topPos = (lineNum - 1) * lineHeight - scrollTop + paddingTop;
        highlighter.style.top = `${topPos}px`;
        highlighter.style.height = `${lineHeight}px`;

        // Auto-scroll if needed
        const visibleTop = textarea.scrollTop;
        const visibleBottom = textarea.scrollTop + textarea.clientHeight;
        const targetTop = (lineNum - 1) * lineHeight;

        if (targetTop < visibleTop || targetTop + lineHeight > visibleBottom) {
          textarea.scrollTop = targetTop - lineHeight * 3; // some padding
        }
      }



    document.getElementById("codeTextarea").addEventListener("scroll", () => {
    document.getElementById("highlightOverlay").scrollTop = document.getElementById("codeTextarea").scrollTop;
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
      let content = await file.text();

    // 🧽 Normalize line endings to Unix style
    content = content.replace(/\r\n/g, '\n');

    // 🚫 Remove BOM if present
    if (content.charCodeAt(0) === 0xFEFF) {
     
        content = content.slice(1);
    }

      // ✅ Populate and enable textarea
      const textarea = document.getElementById("codeTextarea");
      textarea.value = content;
      //textarea.disabled = false;

      // ✅ Load into virtual FS
      if (!Module.FS.analyzePath("/tmp").exists) {
        Module.FS.mkdir("/tmp");
      }

      const filename = "/tmp/manual.s"; // Can use original name if desired
      Module.FS.writeFile(filename, content);

    // 🧪 Optional: Log the sanitized content
    console.log("Sanitized content:", JSON.stringify(content.split("\n")));
    let lineNumber = sim.load(content);
    //logToConsole(`➡️ Line ${lineNumber}`);
    highlightLine(parseInt(lineNumber));
    // ✅ Enable Run and Step buttons
      document.getElementById("runBtn").disabled = false;
      document.getElementById("stepBtn").disabled = false;
      logToConsole(`✅ Loaded ${file.name}`);
    } catch (e) {
      logToConsole("❌ Error loading file: " + e.message);
    }
  };

  // Trigger file input
  document.body.appendChild(input);
  input.click();
  document.body.removeChild(input);
}
document.addEventListener("keydown", (e) => {
  if (e.key === "F10") {
    e.preventDefault(); // ✨ THIS prevents browser focus-stealing
    stepSimulator();
  }
  if (e.key === "F5") {
    e.preventDefault(); // Also avoid accidental page reload
    runSimulator();
  }
});

    document.getElementById("runBtn").addEventListener("click", runSimulator);
    document.getElementById("stepBtn").addEventListener("click", stepSimulator);
    document.getElementById("loadManualBtn").addEventListener("click", loadManualCode);

    window.clearConsole = clearConsole;
