const express = require('express')
const app = express()

const fs = require('fs')
const readline = require('readline');
const Stream = require('stream');

const getLastLine = (fileName, minLength) => {
    let inStream = fs.createReadStream(fileName);
    let outStream = new Stream;
    return new Promise((resolve, reject) => {
        let rl = readline.createInterface(inStream, outStream);

        let lastLine = '';
        rl.on('line', function (line) {
            if (line.length >= minLength) {
                lastLine = line;
            }
        });

        rl.on('error', reject)

        rl.on('close', function () {
            resolve(lastLine)
        });
    })
}

/**
 * @description 
 *  Force light ON or OFF
 *  Values:
 *       0 OFF
 *       1 ON
 */
let status = 0;


app.get('/sensor/status/', (req, res) => {
    return res.send(`${status}`)
})

app.post('/sensor/status', (req, res) => {
    if (status === 0) {
        status = 1
    } else {
        status = 0
    }
    return res.sendStatus(200)
})

app.get('/sensor/tempHum/:tempIndex/:humIndex', (req, res) => {
    const { tempIndex, humIndex } = req.params
    if (tempIndex && humIndex) logTempAndHumIndex(tempIndex, humIndex)
    res.sendStatus(200)
})

app.get('/data/temp', (req, res) => {
    const minLineLength = 1
    getLastLine('./temperatureAndHumidity.log.txt', 1)
        .then((lastLine) => {
            console.log(lastLine)
            const regex = lastLine.match(/TEMP:([0-9][0-3]\.[0-9][0-9])/)
            const data = regex[1]
            return res.send(data)
        })
        .catch((err) => {
            console.error(err)
            return res.send(403)
        })
})

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/index.html")
})

const logTempAndHumIndex = (tempIndex, humIndex) => {
    const currentDate = new Date().toISOString()
    const tempHumLogLine = `[TEMP&HUM]<${currentDate}>TEMP:${tempIndex};HUM:${humIndex}`
    fs.writeFileSync('./temperatureAndHumidity.log.txt', `${tempHumLogLine}\n`, { flag: "a" })
}

app.listen(8080, () => console.log('NODE SERVER RUNNING ON: 8080'))