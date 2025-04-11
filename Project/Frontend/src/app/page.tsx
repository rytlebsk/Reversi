"use client";

import { useEffect, useState } from "react";

import styles from "./page.module.css";

const BOARD_SIZE = 8;

export default function Home() {
  const [selected, setSelected] = useState<string>("");
  const [section, setSection] = useState<string>("");
  const [board, setBoard] = useState<string[][]>([
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "X", "O", "", "", ""],
    ["", "", "", "O", "X", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    // ["", "O", "O", "O", "O", "O", "O", "X"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["O", "O", "O", "O", "O", "O", "O", "O"],
    // ["X", "X", "X", "X", "O", "O", "O", "X"],
  ]);
  const [isAffected, setIsAffected] = useState<string[]>([]);
  const [canPlace, setCanPlace] = useState<string[]>([]);
  const [canPlaceDirs, setCanPlaceDirs] = useState<Record<string, number[][]>>(
    {}
  );
  const [turn, setTurn] = useState<string>("X");
  const [player1, setPlayer1] = useState<string>("Black");
  const [player2, setPlayer2] = useState<string>("White");
  const [timer1, setTimer1] = useState<number>(300);
  const [timer2, setTimer2] = useState<number>(288);
  const [savedGames, setSavedGames] = useState<string[]>(["1", "2", "3"]);

  const [contentType, setContentType] = useState<string>("");

  const handlePlace = (row: number, cell: number, player: string) => {
    setBoard((prev) => {
      const newBoard = [...prev];
      if (newBoard[row][cell]) {
        newBoard[row][cell] = player;
      } else {
        newBoard[row][cell] = player;
      }
      return newBoard;
    });
  };

  const handleNextTurn = () => {
    setTurn(turn === "X" ? "O" : "X");
  };

  const handleCalculateCanPlace = (player: string) => {
    setCanPlace([]);
    setCanPlaceDirs({});
    let newCanPlace: string[] = [];
    let newCanPlaceDirs: Record<string, number[][]> = {};
    for (let i = 0; i < 8; i++) {
      for (let j = 0; j < 8; j++) {
        const cell = board[i][j];
        const dir = [
          [0, 1],
          [1, 1],
          [1, 0],
          [1, -1],
          [0, -1],
          [-1, -1],
          [-1, 0],
          [-1, 1],
        ];
        if (cell === "") continue;
        if (cell === player) continue;
        for (let d of dir) {
          let pos = [i, j];
          while (true) {
            pos[0] += d[0];
            pos[1] += d[1];
            if (i - d[0] < 0 || i - d[0] > BOARD_SIZE - 1) break;
            if (j - d[1] < 0 || j - d[1] > BOARD_SIZE - 1) break;
            if (!!board[i - d[0]][j - d[1]]) break;
            if (pos[0] < 0 || pos[0] > BOARD_SIZE - 1) break;
            if (pos[1] < 0 || pos[1] > BOARD_SIZE - 1) break;
            if (!board[pos[0]][pos[1]]) break;
            if (board[pos[0]][pos[1]] === player) {
              newCanPlace.push(`${i - d[0]}${j - d[1]}`);
              newCanPlaceDirs[`${i - d[0]}${j - d[1]}`] = [
                ...(newCanPlaceDirs[`${i - d[0]}${j - d[1]}`] || []),
                d,
              ];
              break;
            }
          }
        }
      }
    }
    setCanPlace(newCanPlace);
    setCanPlaceDirs(newCanPlaceDirs);
    return newCanPlace;
  };

  const handleAffected = (
    row: number,
    cell: number,
    player: string,
    dirs: number[][]
  ) => {
    let newIsAffected: string[] = [];
    Promise.all(
      dirs.map(async (dir) => {
        let pos = [row, cell];
        while (true) {
          pos[0] += dir[0];
          pos[1] += dir[1];
          if (pos[0] < 0 || pos[0] > BOARD_SIZE - 1) break;
          if (pos[1] < 0 || pos[1] > BOARD_SIZE - 1) break;
          if (!board[pos[0]][pos[1]]) break;
          if (board[pos[0]][pos[1]] === player) break;
          newIsAffected.push(`${pos[0]}${pos[1]}`);
        }
      })
    );
    setIsAffected(newIsAffected);
  };

  const handleFlip = async (
    row: number,
    cell: number,
    player: string,
    dirs: number[][]
  ) => {
    for (let dir of dirs) {
      Promise.all(
        dirs.map(async (dir) => {
          let pos = [row, cell];
          while (true) {
            pos[0] += dir[0];
            pos[1] += dir[1];
            if (pos[0] < 0 || pos[0] > BOARD_SIZE - 1) break;
            if (pos[1] < 0 || pos[1] > BOARD_SIZE - 1) break;
            if (!board[pos[0]][pos[1]]) break;
            if (board[pos[0]][pos[1]] === player) break;
            setBoard((prev) => {
              const newBoard = [...prev];
              newBoard[pos[0]][pos[1]] = player;
              return newBoard;
            });
            await new Promise((resolve) => setTimeout(resolve, 100));
          }
        })
      );
    }
  };

  const handleSetSection = (section: string) => {
    setTimeout(() => setSection(section), 200);
    setTimeout(() => setSelected(""), 800);
    setTimeout(() => setContentType(section), contentType ? 800 : 0);
  };

  useEffect(() => {
    handleCalculateCanPlace(turn);
  }, [board, turn]);

  useEffect(() => {
    if (canPlace.length === 0) {
      handleNextTurn();
    }
  }, [canPlace]);

  return (
    <div className={styles.section} data-section={section}>
      <div className={styles.theme_button} />
      <div className={styles.page}>
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
            onClick={() => {
              setSelected("onlineGame");
              handleSetSection("game");
            }}
          >
            <h2>Online Game</h2>
          </div>
        </div>
      </div>

      <div className={styles.page}>
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
                        if (!canPlace.includes(`${rowIndex}${cellIndex}`))
                          return;
                        handleAffected(
                          rowIndex,
                          cellIndex,
                          turn,
                          canPlaceDirs[`${rowIndex}${cellIndex}`]
                        );
                      }}
                      onMouseLeave={() => {
                        setIsAffected([]);
                      }}
                      onClick={() => {
                        if (!canPlace.includes(`${rowIndex}${cellIndex}`))
                          return;
                        handleFlip(
                          rowIndex,
                          cellIndex,
                          turn,
                          canPlaceDirs[`${rowIndex}${cellIndex}`]
                        );
                        handlePlace(rowIndex, cellIndex, turn);
                        handleNextTurn();
                      }}
                      onDoubleClick={() => {
                        // if (!board[rowIndex][cellIndex]) return;
                        setBoard((prev) => {
                          const newBoard = [...prev];
                          newBoard[rowIndex][cellIndex] = "X";
                          return newBoard;
                        });
                      }}
                    ></div>
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
