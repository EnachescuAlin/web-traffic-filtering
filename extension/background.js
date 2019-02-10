let filteredUrls = [
    //"<all_urls>"
    "http://www.phiorg.ro/*"
];

function sendMessageAndWaitForResponse(message) {
    return new Promise((resolve, reject) => {
        browser.runtime.sendNativeMessage("native_receiver", message)
            .then((response) => {
                resolve(response);
            }, (error) => {
                resolve({ "error": `${error}` });
            })
    });
}

async function onBeforeRequestSendMessage(details) {
    let message = {
        state: "onBeforeRequest",
        requestId: details.requestId
    };
    if (details.documentUrl) {
        message.documentUrl = details.documentUrl;
    }
    if (details.originalUrl) {
        message.originalUrl = details.originalUrl;
    }
    if (details.url) {
        message.url = details.url;
    }
    if (details.method) {
        message.method = details.method;
    }
    if (details.requestBody) {
        message.requestBody = details.requestBody;
    }

    await console.info(`sending data requestId = ${details.requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function onData(_data, _requestId) {
    let message = {
        state: "onData",
        data: _data,
        requestId: _requestId
    };

    await console.info(`sending data requestId = ${_requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function onResponseCompleted(_requestId) {
    let message = {
        state: "onResponseCompleted",
        requestId: _requestId
    };

    await console.info(`sending data requestId = ${_requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function onBeforeSendHeadersSendMessage(details)
{
    let message = {
        state: "onBeforeSendHeaders",
        requestId: details.requestId,
    }
    if (details.requestHeaders) {
        message.requestHeaders = details.requestHeaders;
    }

    await console.info(`sending data requestId = ${details.requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function onHeadersReceivedSendMessage(details)
{
    let message = {
        state: "onHeadersReceived",
        requestId: details.requestId,
    }
    if (details.responseHeaders) {
        message.responseHeaders = details.responseHeaders;
    }
    if (details.statusLine) {
        message.statusLine = details.statusLine;
    }

    await console.info(`sending data requestId = ${details.requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function onBeforeRequestCbk(details) {
    await console.info(`called onBeforeRequestCbk requestId = ${details.requestId}`);

    let requestId = details.requestId;
    let filter = await browser.webRequest.filterResponseData(requestId);
    let decoder = await new TextDecoder("utf-8");
    let encoder = await new TextEncoder();
    let originalData = "";

    await console.info(`onBeforeRequestCbk received details requestId = ${details.requestId}`
        + `, documentUrl = ${details.documentUrl}, originalUrl = ${details.originalUrl}`
        + `, url = ${details.url}, method = ${details.method}, requestBody = ${details.requestBody}`);

    let response = await onBeforeRequestSendMessage(details);
    await console.info(`onBeforeRequestCbk received response = ${JSON.stringify(response)}`);

    // TODO: parse response

    filter.ondata = async function(event) {
        await console.info(`called ondata requestId = ${requestId}`);

        let data = await decoder.decode(event.data, { stream: true });
        await console.info(`received data on requestId = ${requestId}, data = ${data}`);

        originalData = await originalData.concat(data);

        let response = await onData(data, requestId);
        await console.info(`onData received response = ${JSON.stringify(response)}`);

        // TODO: parse response

        await console.info(`returned from ondata requestId = ${requestId}`);
    }

    filter.onstop = async function(event) {
        await console.info(`called onstop requestId = ${requestId}`);

        let data = "modified content";

        let response = await onResponseCompleted(requestId);
        await console.info(`onResponseCompleted received response = ${JSON.stringify(response)}`);

        // TODO: parse response

        await filter.write(encoder.encode(data));
        await filter.close();

        await console.info(`returned from onstop requestId = ${requestId}`);
    }

    await console.info(`returned from onBeforeRequestCbk requestId = ${requestId}`);
    return {};
}

async function onBeforeSendHeadersCbk(details) {
    await console.info(`called onBeforeSendHeadersCbk requestId = ${details.requestId}`);

    let response = await onBeforeSendHeadersSendMessage(details);
    await console.info(`onBeforeSendHeadersCbk received response = ${JSON.stringify(response)}`);

    // TODO: parse response

    await console.info(`returned from onBeforeSendHeadersCbk requestId = ${details.requestId}`);
    return { requestHeaders: details.requestHeaders };
}

async function onHeadersReceivedCbk(details) {
    await console.info(`called onHeadersReceivedCbk requestId = ${details.requestId}`);

    let response = await onHeadersReceivedSendMessage(details);
    await console.info(`onHeadersReceivedCbk received response = ${JSON.stringify(response)}`);

    // TODO: parse response

    await console.info(`returned from onHeadersReceivedCbk requestId = ${details.requestId}`);
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
