"use client";

import { useState } from "react";

import styles from "./page.module.css";

export default function Home() {
  const [selected, setSelected] = useState<string>("");
  const [section, setSection] = useState<string>("");
  const [board, setBoard] = useState<string[][]>([
    // ["", "", "", "", "", "", "", ""],
    // ["", "", "", "", "", "", "", ""],
    // ["", "", "", "", "", "", "", ""],
    // ["", "", "", "X", "O", "", "", ""],
    // ["", "", "", "O", "X", "", "", ""],
    // ["", "", "", "", "", "", "", ""],
    // ["", "", "", "", "", "", "", ""],
    // ["", "", "", "", "", "", "", ""],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
    ["O", "O", "O", "O", "O", "O", "O", "O"],
  ]);
  const [canPlace, setCanPlace] = useState<string[]>(["45", "56", "67"]);
  const [isAffected, setIsAffected] = useState<string[]>(["12", "23", "34"]);
  const [turn, setTurn] = useState<string>("X");
  const [player1, setPlayer1] = useState<string>("TestPlayer1");
  const [player2, setPlayer2] = useState<string>("TestPlayer2");
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

  const handleSetSection = (section: string) => {
    setTimeout(() => setSection(section), 200);
    setTimeout(() => setSelected(""), 800);
    setTimeout(() => setContentType(section), contentType ? 800 : 0);
  };

  return (
    <div className={styles.section} data-section={section}>
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
                      onClick={() => {
                        // if (!canPlace.includes(`${rowIndex}${cellIndex}`))
                        //   return;
                        handlePlace(rowIndex, cellIndex, turn);
                        setTurn(turn === "X" ? "O" : "X");
                      }}
                      onDoubleClick={() => {
                        // if (!board[rowIndex][cellIndex]) return;
                        // setBoard((prev) => {
                        //   const newBoard = [...prev];
                        //   newBoard[rowIndex][cellIndex] = "X";
                        //   return newBoard;
                        // });
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
