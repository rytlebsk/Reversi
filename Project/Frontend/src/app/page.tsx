"use client";

import { useCallback, useEffect, useMemo, useState } from "react";

import styles from "./page.module.css";

const SOCKET_URL = "wss://cloud-backend-aznm.onrender.com/";

export default function Home() {
  const [socket, setSocket] = useState<WebSocket | null>(null);
  const [selected, setSelected] = useState<string>("");
  const [section, setSection] = useState<string>("");
  const [contentType, setContentType] = useState<string>("");

  // Game
  const [board, setBoard] = useState<string[][]>([
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
  ]);
  const [isAffected, setIsAffected] = useState<string[]>([]);
  const [canPlace, setCanPlace] = useState<string[]>([]);
  const [affected, setAffected] = useState<Record<string, string[]>>({});
  const [role, setRole] = useState<"black" | "white" | "">("");
  const [turn, setTurn] = useState<"black" | "white">("black");
  const [player1, setPlayer1] = useState<string>("");
  const [player2, setPlayer2] = useState<string>("");
  const [timer1, setTimer1] = useState<number>(0);
  const [timer2, setTimer2] = useState<number>(0);

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
  const handlePlace = (row: number, cell: number) => {
    socket?.send(JSON.stringify({ event: "place", position: `${row}${cell}` }));
  };

  const handleJoinGame = (
    type?: "local" | "online" | "bot",
    gameId?: string
  ) => {
    if (gameId) {
      socket?.send(JSON.stringify({ event: "join", gameId: gameId }));
    } else if (type === "local") {
      socket?.send(JSON.stringify({ event: "join", gameId: "new_game_l" }));
    } else if (type === "online") {
      socket?.send(JSON.stringify({ event: "join", gameId: "new_game_p" }));
    } else if (type === "bot") {
      socket?.send(JSON.stringify({ event: "join", gameId: "new_game_b" }));
    }
  };

  const handleLeaveGame = () => {
    socket?.send(JSON.stringify({ event: "leave" }));
  };

  const handleClear = () => {
    setBoard([
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
      ["", "", "", "", "", "", "", ""],
    ]);
    setIsAffected([]);
    setCanPlace([]);
    setAffected({});
    setRole("");
    setTurn("black");
    setPlayer1("");
    setPlayer2("");
    setTimer1(0);
    setTimer2(0);
  };

  // const handleNextTurn = () => {
  //   setTurn(turn === "black" ? "white" : "black");
  // };

  const handleAffected = (row: number, cell: number) => {
    setIsAffected(affected[`${row}${cell}`] || []);
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

  const handleFormatTimer = (timer: number) => {
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
      if (turn === "black") setTimer1(Math.max(timer1 - 1, 0));
      if (turn === "white") setTimer2(Math.max(timer2 - 1, 0));
      if (section === "queue") setQueueTimer(Math.max(queueTimer + 1, 0));
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
      console.log("opponent found");
      handleSetSection("game");
    };
    if (section !== "queue") return;
    waitForOpponent();
  }, [section, player2, handleSetSection]);

  useEffect(() => {
    const id = localStorage.getItem("id");
    console.log(id);
    if (id) {
      socket?.send(JSON.stringify({ event: "login", id: id }));
    } else {
      socket?.send(JSON.stringify({ event: "register" }));
    }
  }, [socket]);

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
        case "login":
          // data = {
          //   event:,
          //   id:,
          //   savedGames:,
          // }
          setPlayer1(data.id);
          setSavedGames(data.savedGames);
          setSavedGameStatus(data.done);
          break;
        case "registered":
          // data = {
          //   event:,
          //   id:,
          // }
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
          setRole(data.role);
          break;
        case "sync":
          // data = {
          //   event:,
          //   player1timer:,
          //   player2timer:,
          // }
          setTimer1(data.player1timer);
          setTimer2(data.player2timer);
          break;
        case "update":
          // data = {
          //   event:,
          //   turn:,
          //   board:,
          //   canDo:,
          // }
          setBoard(data.board);
          setCanPlace(Object.keys(data.canDo));
          setAffected(data.canDo);
          setTurn(data.turn);
          break;
        case "left":
          // data = {
          //   event:,
          // }
          setPlayer2("");
          handleSetSection("");
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
            <div className={styles.header}>
              <div className={`${styles.display} ${styles.left} `}>
                <div
                  className={`${styles.player} ${
                    turn === "black" ? styles.turn : ""
                  }`}
                >
                  {player1}
                </div>
                <div className={styles.timer}>{timer1}</div>
              </div>
              <div className={`${styles.display} ${styles.right} `}>
                <div className={styles.timer}>{timer2}</div>
                <div
                  className={`${styles.player} ${
                    turn === "white" ? styles.turn : ""
                  }`}
                >
                  {player2}
                </div>
              </div>
            </div>

            <div className={styles.board}>
              {board.map((row, rowIndex) => (
                <div className={styles.row} key={rowIndex}>
                  {row.map((cell, cellIndex) => (
                    <div
                      key={cellIndex}
                      className={`
                        ${styles.cell}
                        ${cell === "black" ? styles.black : ""} 
                        ${cell === "white" ? styles.white : ""} 
                        ${
                          isAffected.includes(`${rowIndex}${cellIndex}`)
                            ? styles.affected
                            : ""
                        }
                        ${
                          canPlace.includes(`${rowIndex}${cellIndex}`)
                            ? styles.place
                            : ""
                        }
                        ${
                          (rowIndex + cellIndex) % 2 === 0
                            ? styles.even
                            : styles.odd
                        }`}
                      onMouseEnter={() => {
                        handleAffected(rowIndex, cellIndex);
                      }}
                      onMouseLeave={() => {
                        handleResetAffected();
                      }}
                      onClick={() => {
                        if (turn !== role) return;
                        if (!canPlace.includes(`${rowIndex}${cellIndex}`))
                          return;
                        handlePlace(rowIndex, cellIndex);
                      }}
                    />
                  ))}
                </div>
              ))}
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
                  handleLeaveGame();
                }}
              >
                <h2>Back</h2>
              </div>
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
                  Slot {index} {STATUS_CODES[savedGameStatus[index]]}
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
