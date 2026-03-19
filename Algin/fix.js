const fs = require('fs');
function fix(file) {
    let t = fs.readFileSync(file, 'utf8');
    let idx = t.indexOf('6827657117886812795');
    if (idx !== -1) {
        let a_idx = t.indexOf('"a": 1.0', idx);
        if (a_idx !== -1 && a_idx - idx < 1000) {
            t = t.substring(0, a_idx) + '"a": 0.0' + t.substring(a_idx + 8);
            fs.writeFileSync(file, t);
            console.log("Fixed " + file);
        }
    }
}
fix('c:/Users/CheEe.Fang/Desktop/csd3401f25_team_alcosoft/Algin/Algin/Assets/Scenes/TempMap - Copy.scene');
fix('c:/Users/CheEe.Fang/Desktop/csd3401f25_team_alcosoft/Algin/Algin/Assets/Scenes/Level_2.scene');
