import streamlit as st
import yfinance as yf
import pandas as pd
import matplotlib.pyplot as plt


st.set_page_config(
    page_title = "Financial Research",
    layout = "wide"
)

with st.sidebar:
    st.title("Data configuration")
    ticker = st.text_input("Enter a stock ticker", "AAPL")
    period = st.selectbox("Enter a time window", ('1mo', '6mo', '1y'), index = 1)
    button = st.button("Launch analytics")

if button:
    if not ticker.strip():
        st.error("Please, provide a valid ticker")
    else:
        try:
            stock = yf.Ticker(ticker)
            interval = '1h'
            if period == '6mo':
                interval = '1d'
            elif period == '1y':
                interval = '1d'

            stock_data = stock.history(period = period, interval = interval)
            col1, col2 = st.columns(2)

            df = pd.DataFrame(stock_data["Close"])

            with col1:
                col1.header = 'Dataframe'
                col1.dataframe(df, width=500)

            with col2:
                fig, ax = plt.subplots()
                ax.plot(df.index, df.Close)
                st.pyplot(fig)

        except Exception as e:
            st.error(f"An exception occured. Details:{e}")
