// This file works akin to ui.js for the UI, it defines a TypeScript class that
// wraps C++ DISTRHO::Plugin and is meant to be extended by the plugin author.
// The plugin binary then will inject a compiled version of dsp.ts (named
// dsp.wasm) into the Wasmer VM just like ui.js is injected into the web view.

// https://www.assemblyscript.org/editor.html
// https://webassembly.github.io/wabt/demo/wat2wasm/


const instance = createPlugin();

/**
 * Exported functions
 */

export function getLabel(): ArrayBuffer {
  return String.UTF8.encode(instance.getLabel(), true);
}

export function getMaker(): ArrayBuffer {
  return String.UTF8.encode(instance.getMaker(), true);
}

export function getLicense(): ArrayBuffer {
  return String.UTF8.encode(instance.getLicense(), true);
}

/**
 * Public
 */

class DISTRHO_Plugin {

  getLabel(): string {
    abort("getLabel() not implemented", /*filename*/null, /*lineNumber*/0, /*columnNumber*/0); // TO DO
    return "";
  }

  getMaker(): string {
    abort("getMaker() not implemented", /*filename*/null, /*lineNumber*/0, /*columnNumber*/0); // TO DO
    return "";
  }

  getLicense(): string {
    abort("getLicense() not implemented", /*filename*/null, /*lineNumber*/0, /*columnNumber*/0); // TO DO
    return "";
  }

}



/**
 * TODO - this is the user's implementation
 *        it is here to keep things simple during development
 */

function createPlugin(): DISTRHO_Plugin {
  return new WasmTestPlugin;
}

class WasmTestPlugin extends DISTRHO_Plugin {

  getLabel(): string {
    return "WasmTest";
  }

  getMaker(): string {
    return "Luciano Iam";
  }

  getLicense(): string {
    return "ISC";
  }

}
