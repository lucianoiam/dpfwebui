import DPF from './dpf.js';

{
    console.log('Hello stderr from JS');

    const main = document.getElementById('main');
    main.innerHTML = `<h1>Hello World</h1>`;

    const dpf = new DPF;
    dpf.editParameter(123, false);
}
