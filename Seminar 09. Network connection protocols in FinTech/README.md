# Network Connection Protocols in Financial Applications

## TCP and UDP
### TCP in Fintech
TCP is widely used in fintech applications where reliability and data integrity are critical:
1. **Order Execution Systems**. TCP is the standard protocol for sending orders to exchanges and receiving execution reports. Its reliability ensures that every order is accurately transmitted and acknowledged, which is crucial for maintaining the integrity of financial transactions.
2. **Online Banking and Payment Systems**. For secure transactions and account management, TCP's guaranteed delivery and ordered data transmission are essential. This ensures that sensitive financial information is transmitted accurately and completely.
3. **Financial Data Transfer**. When transferring large volumes of financial data, such as market data feeds or batch processing of transactions, TCP's error checking and retransmission capabilities ensure data integrity.

### UDP in Fintech
UDP is employed in scenarios where speed and low latency are prioritized over guaranteed delivery:
1. **Market Data Distribution**. Many market data feeds use multicast UDP to broadcast real-time price updates and order book changes to multiple subscribers simultaneously. This allows for rapid dissemination of market information to high-frequency trading systems.
2. **High-Frequency Trading (HFT)**. While order submission typically uses TCP, some internal components of HFT systems may use UDP for ultra-low latency communication between different parts of the trading infrastructure.
3. **Real-time Analytics**. For applications that process large volumes of financial data in real-time, such as risk management systems or trading algorithms, UDP can be used to stream data quickly, accepting occasional packet loss in favor of lower latency.

### Considerations for Fintech Applications
When choosing between TCP and UDP for fintech applications, several factors must be considered:
1. **Reliability vs. Speed**: TCP ensures data integrity but introduces some latency, while UDP offers lower latency but without guaranteed delivery.
2. **Regulatory Compliance**: Financial regulations often require reliable and auditable data transmission, which may necessitate the use of TCP for certain operations.
3. **Network Conditions**: In high-performance trading environments, UDP may be preferred due to its ability to handle network congestion more efficiently.
4. **Data Criticality**: For mission-critical data where every packet must be received correctly, TCP is the safer choice.
5. **Scalability**: UDP's connectionless nature can be more scalable for applications that need to handle a large number of simultaneous connections.

### Hybrid Approaches
Some fintech systems use a hybrid approach, leveraging both TCP and UDP:
1. Using UDP for initial fast data transmission and TCP for confirmation and error recovery.
2. Employing UDP for real-time data streaming and TCP for order submission and critical updates.

### Emerging Technologies
Advanced technologies are being developed to combine the benefits of both protocols:
1. FPGAs (Field-Programmable Gate Arrays) can be used to optimize TCP implementations, reducing latency while maintaining reliability.
2. Custom protocols built on top of UDP can add reliability features while maintaining lower latency compared to standard TCP.

## The Financial Information eXchange (FIX) protocol
FIX is a standardized electronic communications protocol designed to facilitate the real-time exchange of information related to securities transactions and markets. Developed in 1992, FIX has become the de facto global standard for pre-trade, trade, and post-trade communication in the financial industry.

### Key Features and Applications

FIX is used extensively by buy-side and sell-side firms, trading platforms, exchanges, and even regulators to communicate trade information. Its applications include:
1. Order submissions and changes
2. Trade execution reporting
3. Trade allocations
4. Market data distribution
5. Securities trading
6. Foreign exchange transactions
7. Derivatives trading

The protocol supports various message types, each serving a specific purpose in the trading process. Some common message types include:

1. NewOrderSingle: Used to send a new order to a market
2. ExecutionReport: Used to report the execution of an order
3. OrderCancelRequest: Used to cancel an existing order
4. MarketDataRequest: Used to request market data, such as prices or quotes

### Structure and Functionality

FIX messages are structured using tags and values, where each tag represents a specific piece of data, and the value provides the corresponding information. For example, tag "44" represents the order quantity.

The protocol operates on two main layers:
1. **Session Layer**: Provides reliable, ordered, and recoverable communication between FIX counterparties.
2. **Application Layer**: Specifies the fields and messages used at the application login level.

### Benefits and Impact
The adoption of FIX has brought numerous benefits to the financial industry:
1. **Standardization**: FIX provides a common language for financial institutions, reducing the need for custom interfaces between different systems.
2. **Efficiency**: It has significantly improved transaction speed and reduced the time spent on telephone communications and manual documentation.
3. **Cost Reduction**: The standardized format leads to cost savings in implementation and maintenance.
4. **Flexibility**: FIX can be adapted to meet specific business needs and is continuously updated by the FIX Trading Community.
5. **Global Reach**: It facilitates international trading by providing a universal communication standard.

### FIX Query Example
#### FIX
```FIX
8=FIXT.1.1␁9=271␁35=d␁49=CME␁56=BRKR␁50=CPAPI␁57=user␁1128=9␁320=1234567␁323=100␁48=CS␁22=H␁167=
FUT␁207=NYMEX␁107=WTI Calendar Swap␁969=0.01␁996=Bbl␁997=Mo␁1147=1000␁1227=ENRGY␁1191=Bbl␁1192=1
␁1193=C␁1196=STD␁1198=0␁870=4␁871=29␁872=Y␁871=25␁872=1␁871=24␁872=2␁871=24␁872=12␁964=3400001
␁10=246␁
```

#### FIXML
```FIXML
<?xml version="1.0" encoding="UTF-8" ?>

<FIXML v="FIX50SP2" s="20080115">
    <SecDef RptID="3400001" ReqID="1234567" RspTyp="100">
    <Hdr SID="CME" TID="BRKR" SSub="CPAPI " TSub="user" />
    <Instrmt ID="CS" Src="H" ProdCmplx="ENRGY" SecTyp="FUT" MinPxIncr="0.01"
                UOM="Bbl" UOMQty="1000" PxUOM="Bbl" PxUOMQty="1" SettlMeth="C"
                PxQteMeth="STD" ListMeth="0" TmUnit="Mo" Exch="NYMEX" Desc="
                WTI Calendar Swap" />
    <InstrmtExt>
        <Attrb Typ="29" Val="Y" />
        <Attrb Typ="25" Val="1" />
        <Attrb Typ="24" Val="2" />
        <Attrb Typ="24" Val="12" />
    </InstrmtExt>
    </SecDef>
</FIXML>
```

## Practical Example
TCP Socket-based communication with the pricing engine.
Requirements:
1. [JsonCpp Library](https://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)

Installation of JsonCpp:
```zsh
brew install jsoncpp
```

```bash
apt install libjsoncpp-dev
```

## Extras
* https://www.pico.net/kb/what-are-the-relative-merits-of-tcp-and-udp-in-high-frequency-trading/
* https://www.onixs.biz/fix-protocol.html
* https://ref.onixs.biz/fixml-tutorial.html
* https://www.fixtrading.org/online-specification/introduction/
* https://www.youtube.com/watch?v=32i5TtZao4M
* http://ftp.moex.com/pub/FIX/Spectra/test/docs/back/iqs_fixgate_ru.pdf

