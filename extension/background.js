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

async function onResponseCompleted(_requestId, _data) {
    let message = {
        state: "onResponseCompleted",
        requestId: _requestId,
        data: _data
    };

    await console.info(`sending data requestId = ${_requestId}`
        + `, message = ${JSON.stringify(message)}`);
    return await sendMessageAndWaitForResponse(message);
}

async function parseOnResponseCompletedRsp(response, filter, encoder, originalData) {
    do {
        if (response.error) {
            await console.error(`send onResponseCompleted failed [${response.error}]`);
            break;
        }

        if (!response.status) {
            await console.error(`could not parse response from native receiver`);
            break;
        }

        if (response.status === "allowed") {
            await console.info(`allowed original data`);
            break;
        }

        if (response.status !== "blocked") {
            await console.error(`unknown response status [${response.status}]`);
            break;
        }

        // insert the traffic received from service
        await filter.write(encoder.encode(response.data));
        await filter.close();
        return;
    } while (false);

    // insert the original traffic
    await filter.write(encoder.encode(originalData));
    await filter.close();
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
    let error = false;
    let cancelledRequest = false;

    await console.info(`onBeforeRequestCbk received details requestId = ${details.requestId}`
        + `, documentUrl = ${details.documentUrl}, originalUrl = ${details.originalUrl}`
        + `, url = ${details.url}, method = ${details.method}, requestBody = ${details.requestBody}`);

    filter.ondata = async function(event) {
        await console.info(`called ondata requestId = ${requestId}`);

        let data = await decoder.decode(event.data, { stream: true });
        await console.info(`received data on requestId = ${requestId}, data = ${data}`);

        originalData = await originalData.concat(data);

        await console.info(`returned from ondata requestId = ${requestId}`);
    }

    filter.onstop = async function(event) {
        await console.info(`called onstop requestId = ${requestId}`);

        if (cancelledRequest === true) {
            await console.info('the request was cancelled');
        } else if (error === true) {
            await console.info('error is set => dont send onResponseCompleted to native receiver');

            // insert the original traffic
            await filter.write(encoder.encode(originalData));
            await filter.close();
        } else {
            // send originalData to service
            let response = await onResponseCompleted(requestId, originalData);
            await console.info(`onResponseCompleted received response = ${JSON.stringify(response)}`);

            await parseOnResponseCompletedRsp(response, filter, encoder, originalData);
        }

        await console.info(`returned from onstop requestId = ${requestId}`);
    }

    let ret = {};
    let response = await onBeforeRequestSendMessage(details);
    await console.info(`onBeforeRequestCbk received response = ${JSON.stringify(response)}`);

    if (response.error) {
        await console.error(`onBeforeRequest failed [${response.error}]`);
        error = true;
    } else if (response.status) {
        if (response.status === "cancel") {
            await console.info(`cancelled request`);
            ret.cancel = "true";
            cancelledRequest = true;
        } else if (response.status === "redirect") {
            await console.info(`redirect request to ${response.redirectUrl}`);
            ret.redirectUrl = response.redirectUrl;
        } else if (response.status === "filter") {
            await console.info(`filtering request`);
        } else {
            await console.error(`unknown response status [${response.status}]`);
            error = true;
        }
    } else {
        await console.error(`could not parse response from native receiver`)
        error = true;
    }

    await console.info(`returned from onBeforeRequestCbk requestId = ${requestId}`);
    return ret;
}

async function onBeforeSendHeadersCbk(details) {
    await console.info(`called onBeforeSendHeadersCbk requestId = ${details.requestId}`);

    let ret = { requestHeaders: details.requestHeaders };
    let response = await onBeforeSendHeadersSendMessage(details);
    await console.info(`onBeforeSendHeadersCbk received response = ${JSON.stringify(response)}`);

    if (response.error) {
        await console.error(`onBeforeSendHeaders failed [${response.error}]`);
    } else {
        if (response.status === "allow") {
            await console.info(`allowed the original headers`);
        } else if (response.status === "block") {
            await console.info(`blocked the original headers`);
            if (response.headers) {
                ret = { requestHeaders: response.headers };
            } else {
                await console.info(`response headers is not set`);
            }
        } else {
            await console.error(`unknown response status [${response.status}]`);
        }
    }

    await console.info(`returned from onBeforeSendHeadersCbk requestId = ${details.requestId}`);
    return ret;
}

async function onHeadersReceivedCbk(details) {
    await console.info(`called onHeadersReceivedCbk requestId = ${details.requestId}`);

    let ret = { responseHeaders: details.responseHeaders };
    let response = await onHeadersReceivedSendMessage(details);
    await console.info(`onHeadersReceivedCbk received response = ${JSON.stringify(response)}`);

    if (response.error) {
        await console.error(`onHeadersReceived failed [${response.error}]`);
    } else {
        if (response.status === "allow") {
            await console.info(`allowed the original headers`);
        } else if (response.status === "block") {
            await console.info(`blocked the original headers`);
            if (response.headers) {
                details.responseHeaders = response.headers;
                ret = { responseHeaders: response.headers };
            } else {
                await console.info(`response headers is not set`);
            }
        } else {
            await console.error(`unknown response status [${response.status}]`);
        }
    }

    await console.info(`returned from onHeadersReceivedCbk requestId = ${details.requestId}`);
    return ret;
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
