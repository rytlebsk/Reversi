import { fileURLToPath } from "url";
import { app, BrowserWindow } from "electron";
import serve from "electron-serve";
import net from "net";
import path from "path";
import dotenv from "dotenv";
dotenv.config();

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let mainWindow = null;

let isDev = process.argv.includes("--dev");

let baseUri = isDev ? "http://localhost:3000" : "";

function waitForPort(port) {
  return new Promise((resolve, reject) => {
    let timeout = 30000; // 30 seconds timeout
    let timer;

    function tryConnect() {
      const client = new net.Socket();

      client.once("connect", () => {
        clearTimeout(timer);
        client.destroy();
        resolve();
      });

      client.once("error", () => {
        client.destroy();
        if (timeout <= 0) {
          clearTimeout(timer);
          reject(new Error("Timeout waiting for port"));
          return;
        }
        setTimeout(tryConnect, 1000);
        timeout -= 1000;
      });

      client.connect({ port: port, host: "127.0.0.1" });
    }
    tryConnect();
  });
}

const appServe = app.isPackaged
  ? serve({
      directory: path.join(__dirname, "./out"),
    })
  : !isDev
  ? serve({
      directory: path.join(__dirname, "./out"),
    })
  : null;

const createWindow = async () => {
  if (isDev) {
    try {
      await waitForPort(3000);
    } catch (err) {
      console.error("Failed to connect to Next.js server:", err);
      app.quit();
      return;
    }
  }

  mainWindow = new BrowserWindow({
    width: 1280,
    height: 960,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
    },
  });

  if (app.isPackaged || !isDev) {
    appServe(mainWindow).then(() => {
      mainWindow.loadURL("app://-");
    });
  } else {
    mainWindow.loadURL(`${baseUri}`);
    mainWindow.webContents.openDevTools();
  }
};

app.whenReady().then(createWindow);

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});

app.on("activate", () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
