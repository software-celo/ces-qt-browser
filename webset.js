
var wev
var errorpagepath ="file:///usr/share/minimalbrowser/erl/error.html"


function webEngineConfig(webengine,webEngineView){

	wev=webEngineView;
	wev.loadingChanged.connect(errorPageHandler);
	wev.contextMenuRequested.connect(contextMenuHandler);

	/*###############################################################################################################*/
	/*Cache to Ram Settings*/
	/*Description:
	* Force cache to memory and cookies to memory (RAM)
	* Disable Error Page
	*
	*/

	/*Set new profile settings for webengine*/
	webengine.defaultProfile.httpCacheType = 2;             // No persistent cache
	webengine.defaultProfile.persistentCookiesPolicy = 0;	// No Persistent cookies
	webengine.defaultProfile.offTheRecord = true;           // Hold everything in memory --> disable disk

	if(webengine.defaultProfile.offTheRecord === true){
		console.log("Cache in RAM: Activated")
	}else{
		console.log("Cache in RAM: Deactivated");
		console.log("Please look at the other settings");
		console.log("Storagepath:" + webengine.defaultProfile.persistentStoragePath);
		console.log("CachePath" + webengine.defaultProfile.cachePath);
		console.log("Cachetype:" + webengine.defaultProfile.httpCacheType);
		console.log("Cookiesettings:" + webengine.defaultProfile.persistentCookiesPolicy);
	}
	webengine.defaultProfile.clearHttpCache();
	webengine.defaultProfile.httpCacheMaximumSize = 4000000;
	webengine.settings.errorPageEnabled = false;
	webengine.settings.showScrollBars = false;
}


function blockDialogs(bool_block){

	if(bool_block === true){
		wev.javaScriptDialogRequested.connect(javascriptDialogHandler);
	}
}

function javascriptDialogHandler(request){
	request.accepted = true;
	request.dialogReject();
}


function contextMenuHandler(request){
	/* "Disable" context menu*/
	request.accepted = true;
}

function errorPageHandler(loadRequest){
	switch(loadRequest.status){
		case 0:{
			//console.log("Load started");
			break;
		}
		case 1:{
			//console.log("Load stopped");
			break;
		}
		case 2:{
			//console.log("Load succeded");
			break;
		}
		case 3:{
			console.log("Load failed");
			console.log("Status:" + loadRequest.status);
			console.log("Errordescription:" +  loadRequest.errorString);
			console.log("ErrorDomain:" + loadRequest.errorDomain);
			console.log("ErrorCode:" + loadRequest.errorCode);
			console.log("Requested URL:" + loadRequest.url);
			/*Open standard error page*/
			//webEngineView.url = "file:///ErrorLandingPage/error.html";
			console.log("Go to Errorpage:" + errorpagepath);
			wev.url = errorpagepath +"?edomain=" +loadRequest.errorDomain+"&ecode=" +loadRequest.errorCode ;
			break;
		}
		default:{
		}
	}
}
