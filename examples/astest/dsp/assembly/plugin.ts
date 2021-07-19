import DISTRHO_Plugin from './interface';

export default class AsTestPlugin implements DISTRHO_Plugin {

  getLabel(): string {
    return "AsTest";
  }

  getMaker(): string {
    return "Luciano Iam";
  }

  getLicense(): string {
    return "ISC";
  }

}
