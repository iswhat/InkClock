#!/usr/bin/env node
/**
 * 前端资源构建脚本
 * 用于压缩和合并CSS/JS文件，优化前端性能
 */

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

// 构建配置
const config = {
    css: {
        files: [
            'adminlte/css/adminlte.min.css',
            'css/mobile-adaptation.css'
        ],
        output: 'dist/app.min.css'
    },
    js: {
        files: [
            'adminlte/plugins/jquery/jquery.min.js',
            'adminlte/plugins/bootstrap/js/bootstrap.min.js',
            'adminlte/js/adminlte.min.js',
            'js/menu.js'
        ],
        output: 'dist/app.min.js'
    }
};

// 创建输出目录
function createOutputDir() {
    const distDir = path.join(__dirname, 'dist');
    if (!fs.existsSync(distDir)) {
        fs.mkdirSync(distDir, { recursive: true });
        console.log('Created dist directory');
    }
}

// 读取文件内容
function readFile(filePath) {
    try {
        return fs.readFileSync(path.join(__dirname, filePath), 'utf8');
    } catch (error) {
        console.error(`Error reading file ${filePath}:`, error.message);
        return '';
    }
}

// 写入文件
function writeFile(filePath, content) {
    try {
        fs.writeFileSync(path.join(__dirname, filePath), content);
        console.log(`Written ${filePath}`);
    } catch (error) {
        console.error(`Error writing file ${filePath}:`, error.message);
    }
}

// 压缩CSS
function minifyCSS(css) {
    // 简单的CSS压缩
    return css
        .replace(/\/\*[\s\S]*?\*\//g, '') // 移除注释
        .replace(/\s+/g, ' ') // 合并空白
        .replace(/\s*{\s*/g, '{') // 移除选择器后的空白
        .replace(/\s*}\s*/g, '}') // 移除属性后的空白
        .replace(/\s*:\s*/g, ':') // 移除冒号周围的空白
        .replace(/\s*;\s*/g, ';') // 移除分号周围的空白
        .trim();
}

// 压缩JS
function minifyJS(js) {
    // 简单的JS压缩
    return js
        .replace(/\/\*[\s\S]*?\*\//g, '') // 移除注释
        .replace(/\/\/.*$/gm, '') // 移除单行注释
        .replace(/\s+/g, ' ') // 合并空白
        .replace(/\s*{\s*/g, '{') // 移除大括号前的空白
        .replace(/\s*}\s*/g, '}') // 移除大括号后的空白
        .replace(/\s*;\s*/g, ';') // 移除分号周围的空白
        .replace(/\s*,\s*/g, ',') // 移除逗号周围的空白
        .replace(/\s*\+\s*/g, '+') // 移除加号周围的空白
        .replace(/\s*\-\s*/g, '-') // 移除减号周围的空白
        .replace(/\s*\*\s*/g, '*') // 移除乘号周围的空白
        .replace(/\s*\/\s*/g, '/') // 移除除号周围的空白
        .replace(/\s*\=\s*/g, '=') // 移除等号周围的空白
        .replace(/\s*\?\s*/g, '?') // 移除问号周围的空白
        .replace(/\s*:\s*/g, ':') // 移除冒号周围的空白
        .replace(/\s*&&\s*/g, '&&') // 移除&&周围的空白
        .replace(/\s*\|\|\s*/g, '||') // 移除||周围的空白
        .trim();
}

// 构建CSS
function buildCSS() {
    console.log('Building CSS...');
    
    let combinedCSS = '';
    config.css.files.forEach(file => {
        combinedCSS += readFile(file) + '\n';
    });
    
    const minifiedCSS = minifyCSS(combinedCSS);
    writeFile(config.css.output, minifiedCSS);
    
    // 生成gzip版本
    const gzipCSS = zlib.gzipSync(minifiedCSS);
    writeFile(config.css.output + '.gz', gzipCSS);
    
    console.log('CSS build completed');
}

// 构建JS
function buildJS() {
    console.log('Building JavaScript...');
    
    let combinedJS = '';
    config.js.files.forEach(file => {
        combinedJS += readFile(file) + '\n';
    });
    
    const minifiedJS = minifyJS(combinedJS);
    writeFile(config.js.output, minifiedJS);
    
    // 生成gzip版本
    const gzipJS = zlib.gzipSync(minifiedJS);
    writeFile(config.js.output + '.gz', gzipJS);
    
    console.log('JavaScript build completed');
}

// 主构建函数
function build() {
    console.log('Starting build process...');
    
    createOutputDir();
    buildCSS();
    buildJS();
    
    console.log('Build process completed successfully!');
    console.log('Optimized files:');
    console.log(`- ${config.css.output}`);
    console.log(`- ${config.js.output}`);
}

// 执行构建
if (require.main === module) {
    build();
}

module.exports = { build };