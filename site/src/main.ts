import { Terminal, ITerminalOptions, IFunctionIdentifier } from "xterm";
import { WebsocketBuilder } from "websocket-ts";

import FontFaceObserver from "fontfaceobserver";

import "./styles.scss"

class TerminalWithWebFont extends Terminal {

    loadWebfontAndOpen(element: HTMLElement) {
        const fontFamily = this.options.fontFamily;
        return new FontFaceObserver(fontFamily).load().then(
            () => {
                console.log("Found ", fontFamily);
                this.open(element);
                return this;
            },
            () => {
                console.log("Unable to open font family");
                this.options.fontFamily = "Courier";
                this.open(element);
                return this;
            }
        ); 
    };
};

const terminal = new TerminalWithWebFont({
    fontFamily: "Ubuntu Mono",
    fontSize: 18,
    convertEol: true
} as ITerminalOptions);
 

enum ascii
{
    NUL = 0,
    SOH = 1,
    STX = 2,
    ETX = 3,
    EOT = 4,
    ENQ = 5,
    ACK = 6,
    BEL = 7,
    BS  = 8,
    TAB = 9,
    LF  = 10,
    VT  = 11,
    FF  = 12,
    CR  = 13,
    SO  = 14,
    SI  = 15,
    DLE = 16,
    DC1 = 17,
    DC2 = 18,
    DC3 = 19,
    DC4 = 20,
    NAK = 21,
    SYN = 22,
    ETB = 23,
    CAN = 24,
    EM  = 25,
    SUB = 26,
    ESC = 27,
    Unknown,
    DEL = 127
};

/* Interesting 

https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
https://vt100.net/docs/vt510-rm/contents.html
https://vt100.net/docs/vt510-rm/chapter4.html
*/

terminal.parser.registerOscHandler(2, data => {
    var objs = document.getElementById('window-title') as HTMLElement;
    objs.innerHTML = data;
    // for (var ii = 0; ii != objs.length; ++ii) {
    //     objs[ii].innerHTML = data;
    // }
    return true;
});

terminal.parser.registerDcsHandler({final: 'S'}, 
        (data:string, param:(number|number[])[]) => {
            console.log("Received DCS: ", data);
            if (data == 'serial') {
                let cmaode = document.querySelectorAll<HTMLElement>('#control-mode');
                Array.from(cmaode).forEach(elem => {
                    elem.style.backgroundColor = '#bb3b00';
                    elem.innerHTML = "Serial";
                })
                return true;
            }
            else if (data == 'control') {
                let cmaode = document.querySelectorAll<HTMLElement>('#control-mode');
                Array.from(cmaode).forEach(elem => {
                    elem.style.backgroundColor = '#1ea481';
                    elem.innerHTML = "Control";
                });
                return true;
            }
            return false;
        });

const proto = (document.location.protocol == 'https:' ? 'wss://' : 'ws://')

const ws = new WebsocketBuilder(proto + window.location.host) 
    .onOpen    ( (ws, e) => {
        console.log("opened");
        // const ping = () => {
        //     console.log("ping");
        //     ws.send('\x1b_ping\x1b\\');
        // };
        // setInterval(ping, 5000);

        const port_mode = () => {
            ws.send('\x1b_open_port=1\x1b\\');
        };
        setTimeout(port_mode, 50);
        terminal.focus();
    })
    .onClose   ( (ws, e) => {
        console.log("closed");
        var overlay = document.getElementById("xterm-overlay");
        overlay.style.display = 'flex';
        overlay.innerHTML = 
            '<div class="overlay-message"><p class="overlay-main-message">Disconnected</p>' +
            '<p class="overlay-sub-message">Please refresh the page to continue</p></div>';

        let cmaode = document.querySelectorAll<HTMLElement>('#control-mode');
            Array.from(cmaode).forEach(elem => {
                elem.style.backgroundColor = '#ff4444';
                elem.innerHTML = "Disconnected";
            })
    })
    .onError   ( (ws, e) => console.log("error") )
    .onMessage ( (ws, e) => {
        if (e.data instanceof Blob) {
            e.data.arrayBuffer().then((data:ArrayBuffer) => {
                if (data.byteLength == 1) {
                    const view = new Uint8Array(data);
                    if (view[0] == ascii.BEL) {
                        const audio = document.getElementById('BEL') as HTMLAudioElement;
                        audio.currentTime = 0;
                        audio.play();
                    }
                }
                terminal.write(new TextDecoder('utf-8').decode(data));
            }); 
        } else if (typeof(e.data) == "string") {
            if (e.data.length == 1 && e.data.charCodeAt(0) == ascii.BEL) {
                const audio = document.getElementById('BEL') as HTMLAudioElement;
                audio.currentTime = 0;
                audio.play();
            }
            terminal.write(e.data);
        }
    } )
    .onRetry   ( (ws, e) => console.log("retry") )
    .build();




terminal.loadWebfontAndOpen(document.getElementById("xterm-content"));


terminal.onData((arg1:string, arg2:void) => {
    ws.send(arg1);
})

var upKey = document.getElementById("btn-font-size-up");
upKey?.addEventListener('click', (ev) => {
    terminal.options.fontSize += 1;
})

var downKey = document.getElementById("btn-font-size-down");
downKey?.addEventListener('click', (ev) => {
    var opts = terminal.options;
    opts.fontSize -= 1;
    terminal.options = opts;
})



var upKey = document.getElementById("btn-lines-up");
var rows = terminal.rows;
var cols = terminal.cols;
upKey?.addEventListener('click', (ev) => {
    rows+=1;
    terminal.resize(terminal.cols, rows);
})

var downKey = document.getElementById("btn-lines-down");
downKey?.addEventListener('click', (ev) => {
    // terminal.options.rows -= 1;
    rows -= 1;
    terminal.resize(terminal.cols, rows);
})