"use client";

import { useCallback, useEffect, useMemo, useState } from "react";

import styles from "./page.module.css";

const SOCKET_URL =
  process.env.PUBLIC_SOCKET_URL || "wss://cloud-backend-aznm.onrender.com";

export default function Home() {
  const [id, setId] = useState<string>("");
  const [socket, setSocket] = useState<WebSocket | null>(null);
  const [selected, setSelected] = useState<string>("");
  const [section, setSection] = useState<string>("");
  const [contentType, setContentType] = useState<string>("");

  // Game
  const [board, setBoard] = useState<number[][]>([
    [0, 0, 0, 0, 0, 0, 0, 0], // 0 -> blank
    [0, 0, 0, 0, 0, 0, 0, 0], // 1 -> black
    [0, 0, 0, 0, 0, 0, 0, 0], // 2 -> white
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0],
  ]);
  const [isAffected, setIsAffected] = useState<string[]>([]);
  const [canPlace, setCanPlace] = useState<string[]>([]);
  const [affected, setAffected] = useState<Record<string, string[]>>({});
  const [turn, setTurn] = useState<string>("");
  const [player1, setPlayer1] = useState<string>("");
  const [player2, setPlayer2] = useState<string>("");
  const [timer1, setTimer1] = useState<number>(-1);
  const [timer2, setTimer2] = useState<number>(-1);
  const [replayBoard, setReplayBoard] = useState<number[][][]>([]);
  const [step, setStep] = useState<number>(-1);
  const [result, setResult] = useState<number>(-1);
  const isMyTurn = `${turn}` === `${player1}` || `${turn}` === "0";

  // Load
  const STATUS_CODES = ["Unfinished", "Black Wins", "White Wins", "Tie"];
  const [savedGames, setSavedGames] = useState<string[]>([]);
  const [savedGameStatus, setSavedGameStatus] = useState<number[]>([]);

  // Queue
  const HINTS = useMemo(
    () => [
      "Your move - make it count!",
      "Think ahead. Every piece matters.",
      "Corners win games. Choose wisely.",
      "A single move can turn the tide.",
      "Plan. Flip. Dominate.",
      "Don't rush - strategy beats speed.",
      "Control the board, control the game.",
      "Every flip tells a story.",
      "Watch your opponent. Predict their next step.",
      "Balance offense and defense - stay sharp.",
      "The game is won in the mind, not just on the board.",
      "Timing is everything in chess.",
      "A good move can turn the tide.",
      "A bad move can cost you the game.",
      "Don't be afraid to take risks.",
      "Every move has consequences.",
      "The board is your canvas, paint it well.",
      "Every piece has a story.",
      "The game is a battle of wits.",
      "The best moves are the ones you don't see coming.",
    ],
    []
  );
  const [queueTimer, setQueueTimer] = useState<number>(0);
  const [queueHint, setQueueHint] = useState<string>("");

  // Handlers
  const handlePlace = (cell: number, row: number) => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "place", position: `${cell}${row}` }));
    socket.send(JSON.stringify({ event: "update" }));
  };

  const handleJoinGame = (
    type?: "local" | "online" | "bot",
    gameId?: string
  ) => {
    if (!socket) return;
    if (gameId) {
      socket.send(JSON.stringify({ event: "join", gameId: `${gameId}` }));
    } else if (type === "local") {
      socket.send(JSON.stringify({ event: "join", gameId: "new_game_l" }));
    } else if (type === "online") {
      socket.send(JSON.stringify({ event: "join", gameId: "new_game_p" }));
    } else if (type === "bot") {
      socket.send(JSON.stringify({ event: "join", gameId: "new_game_b" }));
    }
  };

  const handleLeaveGame = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "leave" }));
  };

  const handleLeaveQueue = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "leaveMatch" }));
  };

  const handleUndo = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "undo" }));
  };

  const handleClear = () => {
    setBoard([
      [0, 0, 0, 0, 0, 0, 0, 0], // 0 -> blank
      [0, 0, 0, 0, 0, 0, 0, 0], // 1 -> black
      [0, 0, 0, 0, 0, 0, 0, 0], // 2 -> white
      [0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ]);
    setIsAffected([]);
    setCanPlace([]);
    setAffected({});
    setTurn("");
    setPlayer2("");
    setTimer1(-1);
    setTimer2(-1);
    setStep(-1);
    setResult(-1);
    setReplayBoard([]);
  };

  const handleAffected = (cell: number, row: number) => {
    setIsAffected(affected[`${cell}${row}`] || []);
  };

  const handleResetAffected = () => {
    setIsAffected([]);
  };

  const handleSetSection = useCallback(
    (section: string) => {
      setTimeout(() => setSection(section), 200);
      setTimeout(() => setSelected(""), 800);
      setTimeout(() => setContentType(section), contentType ? 800 : 0);
    },
    [contentType]
  );

  const handleLogin = (id: string) => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "login", id: id }));
  };

  const handleRegister = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "register" }));
  };

  const handleUpdate = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "update" }));
  };

  const handleTimeout = () => {
    if (!socket) return;
    socket.send(JSON.stringify({ event: "timeout" }));
  };

  const handleFormatTimer = (timer: number) => {
    if (timer < 0) return "00:00";
    let minutes = `${Math.floor(timer / 60)}`;
    if (minutes.length === 1) minutes = `0${minutes}`;
    let seconds = `${timer % 60}`;
    if (seconds.length === 1) seconds = `0${seconds}`;
    return `${minutes}:${seconds}`;
  };

  useEffect(() => {
    const loopHint = async () => {
      await new Promise((resolve) => setTimeout(resolve, 1000));
      setQueueHint(HINTS[Math.floor(Math.random() * HINTS.length)]);
      await new Promise((resolve) => setTimeout(resolve, 5000));
      setQueueHint("");
      loopHint();
    };
    loopHint();
  }, [HINTS]);

  useEffect(() => {
    const timer = setInterval(() => {
      if (isMyTurn && timer1 > 0) setTimer1(timer1 - 1);
      if (!isMyTurn && timer2 > 0) setTimer2(timer2 - 1);
      if (section === "queue") setQueueTimer(queueTimer + 1);
      if (timer1 === 0) handleTimeout();
      if (timer2 === 0) handleTimeout();
    }, 1000);
    return () => clearInterval(timer);
  }, [timer1, timer2, queueTimer, turn, section]);

  useEffect(() => {
    if (section !== "queue") return;
    const waitForOpponent = async () => {
      setQueueTimer(0);
      while (player2 === "") {
        await new Promise((resolve) => setTimeout(resolve, 1000));
      }
      handleSetSection("game");
    };
    waitForOpponent();
  }, [section, player2, handleSetSection]);

  useEffect(() => {
    const _id = id || localStorage.getItem("id");
    if (_id) {
      localStorage.setItem("id", _id);
      handleLogin(_id);
    } else {
      handleRegister();
    }
  }, [id, socket]);

  useEffect(() => {
    if (replayBoard.length === 0) return;
    if (step === -1) return;
    setBoard(replayBoard[step]);
  }, [step, replayBoard]);

  useEffect(() => {
    const socket = new WebSocket(SOCKET_URL);

    socket.onopen = () => {
      console.log("connected");
      setSocket(socket);
      handleClear();
    };

    socket.onclose = () => {
      console.log("disconnected");
      setSocket(null);
      handleClear();
    };

    socket.onerror = (event) => {
      console.log("error", event);
    };

    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log(data);
      switch (data.event) {
        case "logined":
          // data = {
          //   event:,
          //   id:,
          //   saveGames:,
          //   dones:,
          // }
          setPlayer1(data.id);
          setSavedGames(data.saveGames);
          setSavedGameStatus(data.dones);
          break;
        case "registered":
          // data = {
          //   event:,
          //   id:,
          // }
          setId(data.id);
          setPlayer1(data.id);
          setSavedGames([]);
          setSavedGameStatus([]);
          localStorage.setItem("id", data.id);
          break;
        case "joined":
          // data = {
          //   event:,
          //   id:,
          //   role:,
          // }
          setPlayer2(data.id);
          handleUpdate();
          break;
        case "replayed":
          // data = {
          //   event:,
          //   board:,
          // }
          setStep(0);
          setReplayBoard(data.board);
          break;
        case "sync":
          // data = {
          //   event:,
          //   timer1:,
          //   timer2:,
          // }
          setTimer1(Math.round(data.timer1));
          setTimer2(Math.round(data.timer2));
          break;
        case "updated":
          // data = {
          //   event:,
          //   player:,
          //   board:,
          //   canDo:,
          // }
          setBoard(data.board);
          setCanPlace(Object.keys(data.canDo));
          setAffected(data.canDo);
          setTurn(data.player);
          break;
        case "left":
          // data = {
          //   event:,
          //   gameId:,
          //   done:,
          // }
          setResult(data.done);
          break;
      }
    };
  }, []);

  return (
    <div className={styles.section} data-section={section}>
      <div className={styles.page}>
        {/* menu */}
        <div className={styles.menu}>
          <div
            className={`${styles.button} ${
              selected === "newGame"
                ? styles.selected
                : selected
                ? styles.unSelected
                : ""
            }`}
            onClick={() => {
              setSelected("newGame");
              handleSetSection("game");
              handleJoinGame("local");
            }}
          >
            <h2>New Game</h2>
          </div>
          <div
            className={`${styles.button} ${
              selected === "newGameBot"
                ? styles.selected
                : selected
                ? styles.unSelected
                : ""
            }`}
            onClick={() => {
              setSelected("newGameBot");
              handleSetSection("game");
              handleJoinGame("bot");
            }}
          >
            <h2>{"New Game (Bot)"}</h2>
          </div>
          <div
            className={`${styles.button} ${
              selected === "loadGame"
                ? styles.selected
                : selected
                ? styles.unSelected
                : ""
            }`}
            onClick={() => {
              setSelected("loadGame");
              handleSetSection("load");
            }}
          >
            <h2>Load Game</h2>
          </div>
          <div
            className={`${styles.button} ${
              selected === "onlineGame"
                ? styles.selected
                : selected
                ? styles.unSelected
                : ""
            }`}
            onClick={async () => {
              setSelected("onlineGame");
              handleSetSection("queue");
              handleJoinGame("online");
            }}
          >
            <h2>Online Game</h2>
          </div>
        </div>
      </div>

      <div className={styles.page}>
        {/* queue */}
        {contentType === "queue" && (
          <div className={styles.queue}>
            <div className={styles.header}>Queue</div>
            <div className={styles.timer}>{handleFormatTimer(queueTimer)}</div>
            <div
              className={`${styles.hint} ${
                queueHint ? styles.intro : styles.outro
              }`}
            >
              <div className={styles.title}>Hint</div>
              <div className={styles.content}>{queueHint}</div>
            </div>
            <div className={styles.footer}>
              <div
                className={`${styles.button} ${
                  selected === "back"
                    ? styles.selected
                    : selected
                    ? styles.unSelected
                    : ""
                }`}
                onClick={() => {
                  setSelected("back");
                  handleSetSection("");
                  handleLeaveQueue();
                }}
              >
                <h2>Back</h2>
              </div>
            </div>
          </div>
        )}

        {/* game */}
        {contentType === "game" && (
          <div className={styles.game}>
            {step === -1 && (
              <div className={styles.header}>
                <div className={`${styles.display} ${styles.left} `}>
                  <div
                    className={`${styles.player} ${
                      isMyTurn ? styles.turn : ""
                    }`}
                  >
                    {player1}
                  </div>
                  <div className={styles.timer}>
                    {handleFormatTimer(timer1)}
                  </div>
                </div>
                <div className={`${styles.display} ${styles.right} `}>
                  <div className={styles.timer}>
                    {handleFormatTimer(timer2)}
                  </div>
                  <div
                    className={`${styles.player} ${
                      !isMyTurn ? styles.turn : ""
                    }`}
                  >
                    {player2}
                  </div>
                </div>
              </div>
            )}

            <div className={styles.board}>
              {board.map((row, rowIndex) => (
                <div className={styles.row} key={rowIndex}>
                  {row.map((cell, cellIndex) => (
                    <div
                      key={cellIndex}
                      className={`
                        ${styles.cell}
                        ${cell === 1 ? styles.x : ""} 
                        ${cell === 2 ? styles.o : ""} 
                        ${
                          isMyTurn &&
                          isAffected.includes(`${cellIndex}${rowIndex}`)
                            ? styles.affected
                            : ""
                        }
                        ${
                          isMyTurn &&
                          canPlace.includes(`${cellIndex}${rowIndex}`)
                            ? styles.place
                            : ""
                        }
                        ${
                          (rowIndex + cellIndex) % 2 === 0
                            ? styles.even
                            : styles.odd
                        }`}
                      onMouseEnter={() => {
                        handleAffected(cellIndex, rowIndex);
                      }}
                      onMouseLeave={() => {
                        handleResetAffected();
                      }}
                      onClick={() => {
                        if (!isMyTurn) return;
                        if (!canPlace.includes(`${cellIndex}${rowIndex}`))
                          return;
                        handlePlace(cellIndex, rowIndex);
                      }}
                    />
                  ))}
                </div>
              ))}
            </div>

            {result !== -1 && (
              <div className={styles.resultText}>
                <h2>{STATUS_CODES[result]}</h2>
              </div>
            )}

            <div className={styles.footer}>
              {step === -1 && (
                <div>
                  <div
                    className={`${styles.button} ${
                      selected === "undo"
                        ? styles.selected
                        : selected
                        ? styles.unSelected
                        : ""
                    }`}
                    onClick={() => handleUndo()}
                  >
                    <h2>Undo</h2>
                  </div>
                  <div
                    className={`${styles.button} ${selected === "back"}`}
                    onClick={() => {
                      setSelected("back");
                      handleSetSection("");
                      handleLeaveGame();
                      setTimeout(() => {
                        handleClear();
                      }, 1000);
                    }}
                  >
                    <h2>Back</h2>
                  </div>
                </div>
              )}
              {step !== -1 && (
                <div>
                  <div
                    className={`${styles.button} ${
                      selected === "prev"
                        ? styles.selected
                        : selected
                        ? styles.unSelected
                        : ""
                    }`}
                    onClick={() => setStep(Math.max(step - 1, 0))}
                  >
                    <h2>Prev</h2>
                  </div>
                  <div
                    className={`${styles.button} ${
                      selected === "next"
                        ? styles.selected
                        : selected
                        ? styles.unSelected
                        : ""
                    }`}
                    onClick={() =>
                      setStep(Math.min(step + 1, replayBoard.length - 1))
                    }
                  >
                    <h2>Next</h2>
                  </div>
                  <div
                    className={`${styles.button} ${
                      selected === "back"
                        ? styles.selected
                        : selected
                        ? styles.unSelected
                        : ""
                    }`}
                    onClick={() => {
                      setSelected("back");
                      handleSetSection("");
                      handleLeaveGame();
                      setTimeout(() => {
                        handleClear();
                      }, 1000);
                    }}
                  >
                    <h2>Back</h2>
                  </div>
                </div>
              )}
            </div>
          </div>
        )}

        {/* load */}
        {contentType === "load" && (
          <div className={styles.load}>
            {savedGames.map((gameId, index) => (
              <div
                key={index}
                className={`${styles.button} ${
                  selected === `save_${gameId}`
                    ? styles.selected
                    : selected
                    ? styles.unSelected
                    : ""
                }`}
                onClick={() => {
                  setSelected(`save_${gameId}`);
                  handleSetSection("game");
                  handleJoinGame("local", gameId);
                }}
              >
                <h2>
                  Game {index + 1} {STATUS_CODES[savedGameStatus[index]]}
                </h2>
              </div>
            ))}

            <div className={styles.footer}>
              <div
                className={`${styles.button} ${
                  selected === "back"
                    ? styles.selected
                    : selected
                    ? styles.unSelected
                    : ""
                }`}
                onClick={() => {
                  setSelected("back");
                  handleSetSection("");
                }}
              >
                <h2>Back</h2>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
