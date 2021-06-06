{
    const main = document.getElementById('main');
    main.innerHTML = `<h1>${window._testString}</h1>`;
    console.log(window._testString);
    window.webviewHost.postMessage(['DPF', 'setParameterValue', 2, 0.5]);
}
