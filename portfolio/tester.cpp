//+------------------------------------------------------------------+
//|                           MartingaleEA-5 Levels.mq5             |
//|                        Copyright 2023, MetaQuotes Ltd.           |
//|                              https://www.mql5.com                |
//+------------------------------------------------------------------+
#property copyright "Copyright 2023, MetaQuotes Ltd."
#property link      "https://www.mql5.com"
#property version   "1.00"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>
#include <Trade\SymbolInfo.mqh>

CTrade trading;
CPositionInfo positioning;
CSymbolInfo symbolInfo;

//***************************Martingale mode**************************//
input bool MartingaleMode = true;
input string Symbol_Name = "EURUSD";                         // Name Of The Symbol
input double MartingaleVolumeMultiplier = 2;                 // Martingale Volume Multiplier
input int MargingaleNum = 5;                                 // Number of Martingale Trades (Max 5 Times)
input double MartingaleDis1 = 300;                           // Distance for Martingale Level 1 (pips)
input double MartingaleDis2 = 400;                           // Distance for Martingale Level 2 (pips)
input double MartingaleDis3 = 500;                           // Distance for Martingale Level 3 (pips)
input double MartingaleDis4 = 600;                           // Distance for Martingale Level 4 (pips)
input double MartingaleDis5 = 700;                           // Distance for Martingale Level 5 (pips)
input double MartingaleTakeProfit = 200;                     // Close All Positions When Total Profit is ($)
input double MartingaleStopLoss = -500;                      // Close All Positions When Total Loss is (-$)
double VOL;

// Lot size normalization
double LotSize = 1.0; // Standard lot size for trading account (modify as per your account)

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
{
    VOL = LotSize * NormalizeDouble(SymbolInfoDouble(Symbol_Name, SYMBOL_VOLUME_MIN), 3);
    return INIT_SUCCEEDED;
}

//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    // Perform any cleanup here if necessary
}

//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
    if (MartingaleMode)
    {
        MartingaleTrading();
        MartingaleClosePos();
        CheckTrades(); // Include CheckTrades function in the tick cycle
    }
}

//+------------------------------------------------------------------+
//| Function to perform Martingale Trading                           |
//+------------------------------------------------------------------+
void MartingaleTrading()
{
    double currentProfit = AccountInfoDouble(ACCOUNT_PROFIT);

    // Check existing positions and place new trades if conditions are met
    for (int i = 0; i < MargingaleNum; ++i)
    {
        bool tradePlaced = false;

        // Check for existing trades
        for (int j = 0; j < PositionsTotal(); ++j)
        {
            if (positioning.SelectByIndex(j) && positioning.Symbol() == Symbol_Name)
            {
                // Check if this position can be martingaled
                if ((positioning.PositionType() == POSITION_TYPE_BUY && positioning.Profit() < 0 && positioning.PriceOpen() - SymbolInfoDouble(Symbol_Name, SYMBOL_BID) >= GetMartingaleDistance(i + 1)) ||
                    (positioning.PositionType() == POSITION_TYPE_SELL && positioning.Profit() < 0 && SymbolInfoDouble(Symbol_Name, SYMBOL_ASK) - positioning.PriceOpen() >= GetMartingaleDistance(i + 1)))
                {
                    double martingaleVol = positioning.Volume() * MartingaleVolumeMultiplier;
                    martingaleVol = NormalizeDouble(martingaleVol, 2);

                    if (positioning.PositionType() == POSITION_TYPE_BUY)
                        trading.Buy(martingaleVol, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 0, 0, "");
                    else if (positioning.PositionType() == POSITION_TYPE_SELL)
                        trading.Sell(martingaleVol, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 0, 0, "");

                    tradePlaced = true;
                    break;
                }
            }
        }

        // If no existing trade is eligible, place a new one
        if (!tradePlaced && currentProfit <= 0)
        {
            if (positioning.PositionType() == POSITION_TYPE_BUY)
                trading.Buy(VOL, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 0, 0, "");
            else if (positioning.PositionType() == POSITION_TYPE_SELL)
                trading.Sell(VOL, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 0, 0, "");

            tradePlaced = true;
        }

        // If no trade placed and total trade value is not positive, break the loop
        if (!tradePlaced && currentProfit > 0)
            break;
    }
}

//+------------------------------------------------------------------+
//| Function to close Martingale positions when reaching profit       |
//+------------------------------------------------------------------+
void MartingaleClosePos()
{
    double totalProfit = 0;

    for (int i = PositionsTotal() - 1; i >= 0; --i)
    {
        if (positioning.SelectByIndex(i))
        {
            if (positioning.Symbol() == Symbol_Name)
            {
                if (positioning.PositionType() == POSITION_TYPE_BUY)
                    totalProfit += positioning.Profit();
                else if (positioning.PositionType() == POSITION_TYPE_SELL)
                    totalProfit -= positioning.Profit();
            }
        }
    }

    if (totalProfit >= MartingaleTakeProfit || totalProfit <= MartingaleStopLoss)
    {
        for (int j = PositionsTotal() - 1; j >= 0; --j)
        {
            if (positioning.SelectByIndex(j))
            {
                if (positioning.Symbol() == Symbol_Name)
                {
                    trading.PositionClose(PositionGetTicket(j), 5);
                }
            }
        }
    }
}

//+------------------------------------------------------------------+
//| Function to check and manage trades based on EMA                   |
//+------------------------------------------------------------------+
void CheckTrades()
{
    double ema50 = iMA(Symbol_Name, PERIOD_M1, 50, MODE_EMA, 0, PRICE_CLOSE, 0);
    double ema200 = iMA(Symbol_Name, PERIOD_M1, 200, MODE_EMA, 0, PRICE_CLOSE, 0);

    for (int i = 0; i < 15; ++i)
    {
        double tradeLotSize = LotSize * pow(1.408, i);

        if (PositionsTotal() == 0)
        {
            if (ema50 > ema200)
                trading.Buy(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 0, 0, "");
            else if (ema50 < ema200)
                trading.Sell(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 0, 0, "");
        }
        else if (PositionsTotal() == 1)
        {
            if (positioning.SelectByIndex(0))
            {
                if (positioning.Symbol() == Symbol_Name)
                {
                    if (positioning.PositionType() == POSITION_TYPE_BUY)
                    {
                        double tradeProfit = positioning.Profit();
                        if (tradeProfit > 0)
                            trading.TrailingStop(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 30 * SymbolInfoDouble(Symbol_Name, SYMBOL_POINT));
                        else
                        {
                            double candleDistance = SymbolInfoDouble(Symbol_Name, SYMBOL_ASK) - positioning.PriceOpen();
                            if (candleDistance > 30 * SymbolInfoDouble(Symbol_Name, SYMBOL_POINT))
                                trading.Buy(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 0, 0, "");
                        }
                    }
                    else if (positioning.PositionType() == POSITION_TYPE_SELL)
                    {
                        double tradeProfit = positioning.Profit();
                        if (tradeProfit < 0)
                            trading.TrailingStop(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 30 * SymbolInfoDouble(Symbol_Name, SYMBOL_POINT));
                        else
                        {
                            double candleDistance = positioning.PriceOpen() - SymbolInfoDouble(Symbol_Name, SYMBOL_BID);
                            if (candleDistance > 30 * SymbolInfoDouble(Symbol_Name, SYMBOL_POINT))
                                trading.Sell(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 0, 0, "");
                        }
                    }
                }
            }
        }
        else if (PositionsTotal() > 1)
        {
            double totalProfit = 0;

            for (int j = PositionsTotal() - 1; j >= 0; --j)
            {
                if (positioning.SelectByIndex(j))
                {
                    if (positioning.Symbol() == Symbol_Name)
                    {
                        if (positioning.PositionType() == POSITION_TYPE_BUY)
                            totalProfit += positioning.Profit();
                        else if (positioning.PositionType() == POSITION_TYPE_SELL)
                            totalProfit -= positioning.Profit();
                    }
                }
            }

            if (totalProfit > 0)
            {
                for (int k = PositionsTotal() - 1; k >= 0; --k)
                {
                    if (positioning.SelectByIndex(k))
                    {
                        if (positioning.Symbol() == Symbol_Name)
                        {
                            trading.TrailingStop(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, positioning.PositionType() == POSITION_TYPE_BUY ? SYMBOL_ASK : SYMBOL_BID), 30 * SymbolInfoDouble(Symbol_Name, SYMBOL_POINT));
                        }
                    }
                }
            }
            else
            {
                if (ema50 > ema200)
                    trading.Buy(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_ASK), 0, 0, "");
                else if (ema50 < ema200)
                    trading.Sell(tradeLotSize, Symbol_Name, SymbolInfoDouble(Symbol_Name, SYMBOL_BID), 0, 0, "");
            }
        }
    }
}

//+------------------------------------------------------------------+
//| Helper function to get Martingale Distance based on level        |
//+------------------------------------------------------------------+
double GetMartingaleDistance(int level)
{
    switch (level)
    {
        case 1: return MartingaleDis1;
        case 2: return MartingaleDis2;
        case 3: return MartingaleDis3;
        case 4: return MartingaleDis4;
        case 5: return MartingaleDis5;
        default: return 0;
    }
}

