import React, { useState, useEffect, useMemo } from 'react';
import Board from './Board.js';
import GuessInput from './GuessInput.js';
import FoundSolutions from './FoundSolutions.js';
import SummaryResults from './SummaryResults.js';
import ToggleGameState from './ToggleGameState.js';
import logo from './logo.png'; // REVISIT for actual logo
import './App.css';

import {GAME_STATE} from './GameState.js';

function App() {

  // const obj = require('./Boggle_Solutions_Endpoint.json');
  const [allSolutions, setAllSolutions] = useState([]);  // solutions from solver
  const [foundSolutions, setFoundSolutions] = useState([]);  // found by user
  const [gameState, setGameState] = useState(GAME_STATE.BEFORE); // Just an enuerator or the three states see below
  const [grid, setGrid] = useState([]);   // the grid
  const [totalTime, setTotalTime] = useState(0);  // total time elapsed
  const [size, setSize] = useState(3);  // selected grid size
  const [game, setGame] = useState({}); // used to hold the MOCK REST ENDPOINTDATA 
  // const myMap = useMemo(() => new Map(Object.entries(obj)), [obj]); // cache this value so that it doesn't have to been refreshed everytime we visit the page.

  useEffect(() => {
    if (gameState === GAME_STATE.IN_PROGRESS) {
        const url = "https://alertregion-passivementor-8000.codio.io/api/game/create/" + size;
        console.log("Fetching game data from:", url);
        fetch(url)
            .then((response) => response.json())
            .then((data) => {
                console.log("Received data:", data);
                setGame(data);
                const s = data.grid.replace(/'/g, '"'); // replace single quotes with double quotes
                setGrid(JSON.parse(s)); // parse JSON string to a 2D array
                setFoundSolutions([]);
            })
            .catch((err) => {
                console.log("Fetch error:", err.message);
            });
    }
}, [gameState, size]);

  // useEffect(() => {
  //           if (gameState === GAME_STATE.IN_PROGRESS) {
  //                     const url = "https://alertregion-passivementor-8000.codio.io/api/game/create/" + size;
  //                       fetch(url)
  //                       .then((response) => response.json())
  //                       .then((data) => {
  //                           setGame(data);
  //                           try {
  //                             const s = data.grid.replace(/'/g, '"'); // replace single quotes with double quotes
  //                             setGrid(JSON.parse(s)); // parse JSON string to a 2D array
  //                           } catch (err) {
  //                             console.error("Error parsing grid data:", err);
  //                             setGrid([]); // Set an empty grid if parsing fails
  //                           }
  //                           setFoundSolutions([]);
  //                       })
  //                       .catch((err) => {
  //                           console.log(err.message);
  //                       });
  //       }
  // }, [gameState, size]);

  const Convert = (s) => {  // convert a string into an array of tokens that are strings
    s = s.replace(/'/g, '');
    s = s.replace('[', '');
    s = s.replace(']', '');
    const tokens = s.split(",") // Split the string into an array of tokens
    .map(token => token.trim()) // Trim each token
    .filter(token => token !== ''); // Remove empty tokens
    return tokens;
}
// useEffect will trigger when the array items in the second argument are
// updated so whenever grid is updated, we will recompute the solutions

useEffect(() => {
     if (typeof game.foundwords !== "undefined") {
        let tmpAllSolutions = Convert(game.foundwords);
        setAllSolutions(tmpAllSolutions);
        }
}, [grid, game.foundwords]);

  function correctAnswerFound(answer) {
    console.log("New correct answer:" + answer);
    setFoundSolutions([...foundSolutions, answer]);
  }

  return (
    <div className="App">
      
        <img src={logo}  width="25%" height="25%" class="logo" alt="Bison Boggle Logo" /> 

        <ToggleGameState gameState={gameState}
                       setGameState={(state) => setGameState(state)} 
                       setSize={(state) => setSize(state)}
                       setTotalTime={(state) => setTotalTime(state)}/>


        { gameState === GAME_STATE.IN_PROGRESS &&
        <div>
          <Board board={grid} />

          <GuessInput allSolutions={allSolutions}
                      foundSolutions={foundSolutions}
                      correctAnswerCallback={(answer) => correctAnswerFound(answer)}/>
          <FoundSolutions headerText="Solutions you've found" words={foundSolutions} />
        </div>
      }
      { gameState === GAME_STATE.ENDED &&
        <div>
          <Board board={grid} />
          <SummaryResults words={foundSolutions} totalTime={totalTime} />
          <FoundSolutions headerText="Missed Words [wordsize > 3]: " words={allSolutions}  />
          
        </div>
      }
    </div>
  );
}

export default App;
