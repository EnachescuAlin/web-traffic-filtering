let filteredUrls = [
    //"<all_urls>"
    "http://www.phiorg.ro/*"
];

function onBeforeRequestCbk(details) {
    console.info(`called onBeforeRequestCbk requestId = ${details.requestId}`);

    let requestId = details.requestId;
    let filter = browser.webRequest.filterResponseData(requestId);
    let decoder = new TextDecoder("utf-8");
    let encoder = new TextEncoder();

    /*
     * send documentUrl, originUrl, url, method and requestBody to native app
     */

    filter.ondata = event => {
        console.info(`called ondata requestId = ${requestId}`);

        let data = decoder.decode(event.data, { stream: true });
        console.info(`received data on requestId = ${requestId}, data = ${data}`);

        /*
         * send response's body to native app
         */

        console.info(`returned from ondata requestId = ${requestId}`);
    }

    filter.onstop = event => {
        console.info(`called onstop requestId = ${requestId}`);

        let data = "modified content";
        /*
         * get response's body from native app
         */

        filter.write(encoder.encode(data));
        filter.close();

        console.info(`returned from onstop requestId = ${requestId}`);
    }

    console.info(`returned from onBeforeRequestCbk requestId = ${requestId}`);
    return {};
}

function onBeforeSendHeadersCbk(details) {
    console.info(`called onBeforeSendHeadersCbk requestId = ${details.requestId}`);

    /*
     * send request's headers to native app and wait for response
     */

    console.info(`returned from onBeforeSendHeadersCbk requestId = ${details.requestId}`);
    return { requestHeaders: details.requestHeaders };
}

function onHeadersReceivedCbk(details) {
    console.info(`called onHeadersReceivedCbk requestId = ${details.requestId}`);

    /*
     * send statusLine and response's headers to native app and wait for response
     */

    console.info(`returned from onHeadersReceivedCbk requestId = ${details.requestId}`);
    return { responseHeaders: details.responseHeaders };
}

browser.webRequest.onBeforeRequest.addListener(
    onBeforeRequestCbk,
    {
        urls: filteredUrls,
    },
    [
        "blocking",
        "requestBody"
    ]
);

browser.webRequest.onBeforeSendHeaders.addListener(
    onBeforeSendHeadersCbk,
    {
        urls: filteredUrls
    },
    [
        "blocking",
        "requestHeaders"
    ]
);

browser.webRequest.onHeadersReceived.addListener(
    onHeadersReceivedCbk,
    {
        urls: filteredUrls
    },
    [
        "blocking",
        "responseHeaders"
    ]
);
