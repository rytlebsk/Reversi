"use client";

import { useEffect, useState } from "react";

import styles from "./page.module.css";

export default function Home() {
  const [id, setId] = useState<string>("");
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
  const [turn, setTurn] = useState<string>("X");
  const [player1, setPlayer1] = useState<string>("");
  const [player2, setPlayer2] = useState<string>("");
  const [timer1, setTimer1] = useState<number>(300);
  const [timer2, setTimer2] = useState<number>(288);

  // Load
  const [savedGames, setSavedGames] = useState<string[]>(["1", "2", "3"]);

  // Queue
  const hint: string[] = [
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
  ];
  const [queueTimer, setQueueTimer] = useState<number>(0);
  const [queueHint, setQueueHint] = useState<string>("");

  // Handlers
  const handlePlace = (row: number, cell: number) => {
    socket?.send(JSON.stringify({ event: "place", position: `${row}${cell}` }));
  };

  const handleNextTurn = () => {
    setTurn(turn === "X" ? "O" : "X");
  };

  const handleAffected = (row: number, cell: number) => {
    setIsAffected(affected[`${row}${cell}`] || []);
  };

  const handleResetAffected = () => {
    setIsAffected([]);
  };

  const handleSetSection = (section: string) => {
    setTimeout(() => setSection(section), 200);
    setTimeout(() => setSelected(""), 800);
    setTimeout(() => setContentType(section), contentType ? 800 : 0);
  };

  const handleFormatTimer = (timer: number) => {
    let minutes = `${Math.floor(timer / 60)}`;
    if (minutes.length === 1) minutes = `0${minutes}`;
    let seconds = `${timer % 60}`;
    if (seconds.length === 1) seconds = `0${seconds}`;
    return `${minutes}:${seconds}`;
  };

  // Effects
  // useEffect(() => {
  //   handleCalculateCanPlace(turn);
  // }, [board, turn]);

  // useEffect(() => {
  //   if (canPlace.length === 0) {
  //     handleNextTurn();
  //   }
  // }, [canPlace]);

  useEffect(() => {
    const loopHint = async () => {
      await new Promise((resolve) => setTimeout(resolve, 1000));
      setQueueHint(hint[Math.floor(Math.random() * hint.length)]);
      await new Promise((resolve) => setTimeout(resolve, 5000));
      setQueueHint("");
      loopHint();
    };
    loopHint();
  }, []);

  useEffect(() => {
    if (!id) {
      socket?.send(JSON.stringify({ event: "register" }));
    } else {
      socket?.send(JSON.stringify({ event: "login", id: id }));
      setPlayer1(id);
    }
  }, [id, socket]);

  useEffect(() => {
    const socket = new WebSocket(`ws://localhost:8080`);

    socket.onopen = () => {
      console.log("connected");
      setSocket(socket);
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
          break;
        case "registered":
          // data = {
          //   event:,
          //   id:,
          // }
          setId(data.id);
          break;
        case "joined":
          // data = {
          //   event:,
          //   id:,
          // }
          setPlayer2(data.id);
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
          //   board:,
          //   canDo:,
          // }
          setBoard(data.board);
          setCanPlace(Object.keys(data.canDo));
          setAffected(data.canDo);
          break;
      }
    };
  }, [socket]);

  useEffect(() => {
    const timer = setInterval(() => {
      if (turn === "X") setTimer1(Math.max(timer1 - 1, 0));
      if (turn === "O") setTimer2(Math.max(timer2 - 1, 0));
      if (section === "queue") setQueueTimer(Math.max(queueTimer + 1, 0));
    }, 1000);
    return () => clearInterval(timer);
  }, [timer1, timer2, queueTimer, turn, section]);

  useEffect(() => {
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
  }, [section, player2]);

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
            }}
          >
            <h2>New Game</h2>
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
                    turn === "X" ? styles.turn : ""
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
                    turn === "O" ? styles.turn : ""
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
                        ${cell === "X" ? styles.x : ""} 
                        ${cell === "O" ? styles.o : ""} 
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
                        if (!canPlace.includes(`${rowIndex}${cellIndex}`))
                          return;
                        handlePlace(rowIndex, cellIndex);
                        handleNextTurn();
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
            {savedGames.map((game, index) => (
              <div
                key={index}
                className={`${styles.button} ${
                  selected === `save_${game}`
                    ? styles.selected
                    : selected
                    ? styles.unSelected
                    : ""
                }`}
                onClick={() => {
                  setSelected(`save_${game}`);
                  handleSetSection("game");
                }}
              >
                <h2>Slot {index}</h2>
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
