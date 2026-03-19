const fs = require('fs');
try {
    const content = fs.readFileSync(process.argv[2], 'utf8');
    JSON.parse(content);
    console.log('OK');
} catch (e) {
    console.error(e.message);
}
