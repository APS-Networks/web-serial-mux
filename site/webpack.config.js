/* eslint-disable no-undef */
// const ESLintPlugin = require('eslint-webpack-plugin');

var path = require('path');

const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const loader = require('sass-loader');
const devMode = process.env.NODE_ENV !== 'production';

const nodeModulesPath = path.resolve(__dirname, 'node_modules');
const xtermPath = path.resolve(nodeModulesPath, 'xterm/')
const xtermCSS = path.resolve(xtermPath, 'css/xterm.css')

console.log("NODEMODULES:", nodeModulesPath);
console.log("XTERMPATH:  ", xtermPath);
console.log("XTERMCSS:   ", xtermCSS);
// const xt = require("xterm");


module.exports = {
    plugins: [
        // new MiniCssExtractPlugin({
        //     filename: devMode ? '[name].css' : '[name].[contenthash].css',
        //     chunkFilename: devMode ? '[id].css' : '[id].[contenthash].css',
        // }),
        new MiniCssExtractPlugin()
    ],

    resolve: {
        // alias: {
        //     '~': path.resolve('./node_modules')
        // }
        // extensions: ['', '.js', '.css', '.scss'],
        alias: {
            'xterm-css': xtermCSS
        }
    },
    entry: './src/main.ts',
    output: {
        path: __dirname + '/dist/assets/bundle',
        filename: 'bundle.js'
    },
    module: {
        rules: [
            {
                test: /\.(s(a|c)ss)$/,
                use: [
                    // 'style-loader',
                    MiniCssExtractPlugin.loader,
                    {
                        loader: 'css-loader',
                        options: {
                            url: {
                                filter: (url, resourcePath) => {
                                    if (url.includes('assets')) {
                                        return false;
                                    }
                                    return true;
                                }
                            }
                        }
                    }, 'sass-loader'],
                
            },
            // {
            //     test: /\.ttf$/,
            //     use: [
            //         {
            //             loader: 'file-loader',
            //             options: {
            //                 name: '[name].[ext]',

            //             }
            //         }
            //     ]
            // },
            // {
            //     test: /\.css$/,
            //     include: xtermPath,
            //     use: [ 
            //         MiniCssExtractPlugin.loader,
            //         "css-loader",
            //         {
            //             loader: "sass-loader",
            //             options: {
            //               sassOptions: {
            //                 includePaths: [nodeModulesPath],
            //               },
            //             },
            //           },
            //     ]
            // },
            // {
            //     test: /\.s[ac]ss$/i,
            //     use: [
            //         // Creates `style` nodes from JS strings
            //           "style-loader",
            //         // MiniCssExtractPlugin.loader,
            //         // Translates CSS into CommonJS
            //         "css-loader",
            //         // Compiles Sass to CSS
            //         "sass-loader",
            //     ],
            //   },
            { test: /\.ts$/, use: 'ts-loader', exclude: /node_modules/ },
        ]
    }
}


