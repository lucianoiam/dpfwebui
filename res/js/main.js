{
    const main = document.getElementById('main');
    main.innerHTML = `<h1>${window._testString}</h1>`;
    console.log(window._testString); // window.webkit.messageHandlers.console_log
    //window.webkit.messageHandlers.DPF.postMessage(['setParameterValue', 2, 0.5]);
    //console.log(window.chrome.webiew);
}
