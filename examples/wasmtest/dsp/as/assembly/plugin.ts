import DISTRHO_Plugin from './interface';

export default class Plugin implements DISTRHO_Plugin {

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
