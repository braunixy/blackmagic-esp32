<script>
  import { onMount, tick } from "svelte";
  import ButtonInline from "./ButtonInline.svelte";
  import Spinner from "./Spinner.svelte";

  export let server = "";
  export let api_post;

  let terminal_output = "";
  let current_cmd = "";
  let is_running = false;
  let output_element;
  let input_element;

  let history = [];
  let history_idx = -1;

  const initial_motd = 
`==================================================
 BLACK MAGIC PROBE - WEB TERMINAL
 Hardware: XIAO ESP32-C6 | Target: SWD (D0/D1/D2)
==================================================
Type 'help' or click any quick-action button below.
`;

  terminal_output = initial_motd;

  async function scroll_to_bottom() {
    await tick();
    if (output_element) {
      output_element.scrollTop = output_element.scrollHeight;
    }
  }

  export async function run_command(cmd) {
    if (!cmd || cmd.trim() === "") return;
    const trimmed = cmd.trim();

    if (trimmed.toLowerCase() === "clear") {
      terminal_output = "";
      current_cmd = "";
      return;
    }

    if (history.length === 0 || history[history.length - 1] !== trimmed) {
      history.push(trimmed);
    }
    history_idx = -1;

    terminal_output += `\n>: ${trimmed}\n`;
    current_cmd = "";
    is_running = true;
    await scroll_to_bottom();

    try {
      const res = await api_post(server + "/api/v1/terminal/run", { cmd: trimmed });
      if (res && res.output) {
        terminal_output += res.output;
      } else if (res && res.error) {
        terminal_output += `[Error: ${res.error}]\n`;
      } else {
        terminal_output += "[OK - No output]\n";
      }
    } catch (e) {
      terminal_output += `[Network/HTTP Error: ${e.message}]\n`;
    } finally {
      is_running = false;
      await scroll_to_bottom();
      if (input_element) {
        input_element.focus();
      }
    }
  }

  function handle_keydown(e) {
    if (e.key === "Enter") {
      run_command(current_cmd);
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      if (history.length > 0) {
        if (history_idx === -1) {
          history_idx = history.length - 1;
        } else if (history_idx > 0) {
          history_idx--;
        }
        current_cmd = history[history_idx];
      }
    } else if (e.key === "ArrowDown") {
      e.preventDefault();
      if (history_idx !== -1) {
        if (history_idx < history.length - 1) {
          history_idx++;
          current_cmd = history[history_idx];
        } else {
          history_idx = -1;
          current_cmd = "";
        }
      }
    }
  }

  function clear_terminal() {
    terminal_output = "";
    if (input_element) input_element.focus();
  }

  onMount(() => {
    scroll_to_bottom();
    if (input_element) input_element.focus();
  });
</script>

<div class="terminal-container">
  <!-- Quick action toolbar -->
  <div class="quick-bar">
    <span class="bar-title">QUICK:</span>
    <ButtonInline value="[SWD Scan]" on:click={() => run_command("swd_scan")} />
    <ButtonInline value="[Target Reset]" on:click={() => run_command("swd_reset")} />
    <ButtonInline value="[Targets]" on:click={() => run_command("targets")} />
    <ButtonInline value="[Attach]" on:click={() => run_command("attach")} />
    <ButtonInline value="[Regs]" on:click={() => run_command("regs")} />
    <ButtonInline value="[Flash]" on:click={() => run_command("flash")} />
    <ButtonInline value="[Fault Info]" on:click={() => run_command("fault")} />
    <ButtonInline value="[Halt]" on:click={() => run_command("halt")} />
    <ButtonInline value="[Resume]" on:click={() => run_command("resume")} />
    <ButtonInline value="[Step]" on:click={() => run_command("step")} />
    <ButtonInline value="[GDB Info]" on:click={() => run_command("gdb_info")} />
    <ButtonInline value="[Help]" on:click={() => run_command("help")} />
    <ButtonInline style="normal" value="[Clear]" on:click={clear_terminal} />
  </div>

  <!-- Screen output -->
  <div class="screen-wrapper">
    <pre class="screen" bind:this={output_element}>{terminal_output}</pre>
  </div>

  <!-- Input command line -->
  <div class="input-row">
    <span class="prompt">&gt;:</span>
    <input
      type="text"
      class="cmd-input"
      bind:this={input_element}
      bind:value={current_cmd}
      on:keydown={handle_keydown}
      placeholder="Type command ('help', 'swd_scan', 'reset')..."
      disabled={is_running}
      autocorrect="off"
      autocapitalize="none"
      autocomplete="off"
      spellcheck="false"
    />
    {#if is_running}
      <div class="status-spinner"><Spinner /></div>
    {:else}
      <button class="run-btn" on:click={() => run_command(current_cmd)}>[SEND]</button>
    {/if}
  </div>
</div>

<style>
  .terminal-container {
    display: flex;
    flex-direction: column;
    width: 100%;
    margin-top: 5px;
    box-sizing: border-box;
  }

  .quick-bar {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 6px;
    margin-bottom: 8px;
    padding-bottom: 6px;
    border-bottom: 2px dashed #000;
  }

  .bar-title {
    font-size: 22px;
    font-weight: bold;
    margin-right: 4px;
  }

  .screen-wrapper {
    position: relative;
    width: 100%;
    background-color: #0b0c10;
    border: 3px solid #000;
    box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.8);
    box-sizing: border-box;
  }

  .screen {
    margin: 0;
    padding: 12px;
    height: 340px;
    overflow-y: auto;
    overflow-x: hidden;
    color: #ffa21c;
    font-family: "DOS", monospace;
    font-size: 20px;
    line-height: 1.3;
    white-space: pre-wrap;
    word-break: break-all;
    user-select: text;
    -webkit-user-select: text;
  }

  .screen::-webkit-scrollbar {
    width: 10px;
  }

  .screen::-webkit-scrollbar-track {
    background: #000;
  }

  .screen::-webkit-scrollbar-thumb {
    background: #ffa21c;
  }

  .input-row {
    display: flex;
    align-items: center;
    margin-top: 8px;
    gap: 8px;
    width: 100%;
    box-sizing: border-box;
  }

  .prompt {
    font-size: 26px;
    font-weight: bold;
    color: #000;
  }

  .cmd-input {
    flex: 1;
    font-family: "DOS", monospace;
    font-size: 22px;
    height: 38px;
    padding: 2px 8px;
    border: 2px solid #000;
    background-color: #fff;
    color: #000;
    box-sizing: border-box;
  }

  .cmd-input:focus {
    outline: none;
    background-color: #fff;
    border-color: #000;
    box-shadow: 2px 2px 0 #000;
  }

  .run-btn {
    font-family: "DOS", monospace;
    font-size: 22px;
    height: 38px;
    padding: 0 12px;
    background-color: #000;
    color: #fff;
    border: none;
    cursor: pointer;
    box-sizing: border-box;
  }

  .run-btn:hover {
    background-color: #fff;
    color: #000;
    outline: 2px solid #000;
  }

  .status-spinner {
    padding: 0 10px;
  }

  @media (max-width: 600px) {
    .screen {
      height: 260px;
      font-size: 16px;
    }
    .cmd-input {
      font-size: 18px;
    }
    .run-btn {
      font-size: 18px;
    }
  }
</style>
