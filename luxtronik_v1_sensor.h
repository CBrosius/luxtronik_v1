#include "esphome.h"

namespace esphome {

const char ASCII_CR = 0x0D;
const char ASCII_LF = 0x0A;

const uint8_t READ_BUFFER_LENGTH = 255;

static const char* TAG = "luxtronik_v1";

class luxtronik_v1_sensor : public PollingComponent, public uart::UARTDevice{
 public:
  // constructor
  luxtronik_v1_sensor(UARTComponent *parent) : PollingComponent(60000), UARTDevice(parent) {}

  // Temperatures
  Sensor *temp_VL           = new Sensor();  // Temperatur Vorlauf  -> 1100/2
  Sensor *temp_RL           = new Sensor();  // Temperatur Rücklauf -> 1100/3
  Sensor *temp_RL_Soll      = new Sensor();  // Temperatur Rücklauf-Soll -> 1100/4
  Sensor *temp_Heissgas     = new Sensor();  // 1100/5
  Sensor *temp_Aussen       = new Sensor();  // 1100/6
  Sensor *temp_BW           = new Sensor();  // 1100/7
  Sensor *temp_BW_Soll      = new Sensor();  // 1100/8
  Sensor *temp_WQ_Ein       = new Sensor();  // 1100/9
  Sensor *temp_Kaeltekreis  = new Sensor();  // 1100/10
  Sensor *temp_MK1_Vorl     = new Sensor();  // 1100/11
  Sensor *temp_MK1VL_Soll   = new Sensor();  // 1100/12
  Sensor *temp_Raumstat     = new Sensor();  // 1100/13
  // Inputs
  Sensor *ein_Abtau_Soledruck_Durchfluss  = new Sensor();  // 1200/2
  Sensor *ein_Sperrzeit_EVU               = new Sensor();  // 1200/3
  Sensor *ein_Hochdruckpressostat         = new Sensor();  // 1200/4
  Sensor *ein_Motorschutz                 = new Sensor();  // 1200/5
  Sensor *ein_Niederdruckpressostat       = new Sensor();  // 1200/6
  Sensor *ein_Fremdstromanode             = new Sensor();  // 1200/7
  // Outputs
  Sensor *aus_ATV             = new Sensor();  // 1300/2
  Sensor *aus_BWP             = new Sensor();  // 1300/3
  Sensor *aus_FBHP            = new Sensor();  // 1300/4
  Sensor *aus_HZP             = new Sensor();  // 1300/5
  Sensor *aus_Mischer_1_Auf   = new Sensor();  // 1300/6
  Sensor *aus_Mischer_1_Zu    = new Sensor();  // 1300/7
  Sensor *aus_VentWP          = new Sensor();  // 1300/8
  Sensor *aus_VentBrunnen     = new Sensor();  // 1300/9
  Sensor *aus_Verdichter_1    = new Sensor();  // 1300/10
  Sensor *aus_Verdichter_2    = new Sensor();  // 1300/11
  Sensor *aus_ZPumpe          = new Sensor();  // 1300/12
  Sensor *aus_ZWE             = new Sensor();  // 1300/13
  Sensor *aus_ZWE_Stoerung    = new Sensor();  // 1300/14
  // Status
  Sensor *status_Anlagentyp          = new Sensor();  // 1700/2
  Sensor *status_Softwareversion     = new Sensor();  // 1700/3
  Sensor *status_Bivalenzstufe       = new Sensor();  // 1700/4
  Sensor *status_Betriebszustand     = new Sensor();  // 1700/5
  // Sensor *status_Startdatum_Tag      = new Sensor();  // 1700/6
  // Sensor *status_Startdatum_Monat    = new Sensor();  // 1700/7
  // Sensor *status_Startdatum_Jahr     = new Sensor();  // 1700/8
  // Sensor *status_Startuhrzeit_Std    = new Sensor();  // 1700/9
  // Sensor *status_Startuhrzeit_Min    = new Sensor();  // 1700/10
  // Sensor *status_Startuhrzeit_Sek    = new Sensor();  // 1700/11
  // Sensor *status_Compact             = new Sensor();  // 1700/12
  // Sensor *status_Comfort             = new Sensor();  // 1700/13
  // Modi
  Sensor *modus_Heizung     = new Sensor();  // 3405/2
  Sensor *modus_Warmwasser  = new Sensor();  // 3505/2

  void loop() override{
    // Read message
    while (this->available()) {
        uint8_t byte;
        this->read_byte(&byte);

        if (this->read_pos_ == READ_BUFFER_LENGTH)
        this->read_pos_ = 0;

        ESP_LOGVV(TAG, "Buffer pos: %u %d", this->read_pos_, byte);  // NOLINT

        if (byte == ASCII_CR)
        continue;
        if (byte >= 0x7F)
        byte = '?';  // need to be valid utf8 string for log functions.
        this->read_buffer_[this->read_pos_] = byte;

        if (this->read_buffer_[this->read_pos_] == ASCII_LF) {
        this->read_buffer_[this->read_pos_] = 0;
        this->read_pos_ = 0;
        this->parse_cmd_(this->read_buffer_);
        } else {
        this->read_pos_++;
        }
    }
  }

  void update() override {
    // Ask for Temperatures
    send_cmd_("1100");
  }

 protected:

  std::string sender_;
  char read_buffer_[READ_BUFFER_LENGTH];
  size_t read_pos_{0};

  float GetFloatTemp(std::string message){
    return std::atof(message.c_str())/10;
  }

  float GetInputOutputState(std::string message){
    return std::atoi(message.c_str());
  }

  void send_cmd_(std::string message){
    ESP_LOGD(TAG, "S: %s - %d", message.c_str(), 0);
    this->write_str(message.c_str());
    this->write_byte(ASCII_CR);
    this->write_byte(ASCII_LF);
  }

  void parse_cmd_(std::string message){
    // ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
    std::string delimiter = ";";

    if ((message.find("1100")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "1100 found -> Temperatures");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // ESP_LOGD(TAG, "Length: %d", message.length());
        // 1100;12;254;257;261;456;128;480;470;177;201;750;0;0
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_VL           = new Sensor();     // Temperatur Vorlauf  -> 1100/2 
        std::string tmp_VL = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_VL.c_str());
        temp_VL->publish_state(GetFloatTemp(tmp_VL));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_RL           = new Sensor();     // Temperatur Rücklauf -> 1100/3 
        std::string tmp_RL = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_RL.c_str());
        temp_RL->publish_state(GetFloatTemp(tmp_RL));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_RL_Soll      = new Sensor();     // Temperatur Rücklauf-Soll -> 1100/4 
        std::string tmp_RL_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_RL_Soll.c_str());
        temp_RL_Soll->publish_state(GetFloatTemp(tmp_RL_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Heissgas     = new Sensor();  // 1100/5 
        std::string tmp_Heissgas = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Heissgas.c_str());
        temp_Heissgas->publish_state(GetFloatTemp(tmp_Heissgas));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Aussen       = new Sensor();  // 1100/6 
        std::string tmp_Aussen = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Aussen.c_str());
        temp_Aussen->publish_state(GetFloatTemp(tmp_Aussen));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_BW       = new Sensor();  // 1100/7 
        std::string tmp_BW = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_BW.c_str());
        temp_BW->publish_state(GetFloatTemp(tmp_BW));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_BW_Soll      = new Sensor();  // 1100/8 
        std::string tmp_BW_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_BW_Soll.c_str());
        temp_BW_Soll->publish_state(GetFloatTemp(tmp_BW_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_WQ_Ein       = new Sensor();  // 1100/9 
        std::string tmp_WQ_Ein = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_WQ_Ein.c_str());
        temp_WQ_Ein->publish_state(GetFloatTemp(tmp_WQ_Ein));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Kaeltekreis  = new Sensor();  // 1100/10
        std::string tmp_Kaeltekreis = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Kaeltekreis.c_str());
        temp_Kaeltekreis->publish_state(GetFloatTemp(tmp_Kaeltekreis));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_MK1_Vorl     = new Sensor();  // 1100/11
        std::string tmp_MK1_Vorl = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_MK1_Vorl.c_str());
        temp_MK1_Vorl->publish_state(GetFloatTemp(tmp_MK1_Vorl));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_MK1VL_Soll   = new Sensor();  // 1100/12
        std::string tmp_MK1VL_Soll = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_MK1VL_Soll.c_str());
        temp_MK1VL_Soll->publish_state(GetFloatTemp(tmp_MK1VL_Soll));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *temp_Raumstat     = new Sensor();  // 1100/13
        std::string tmp_Raumstat = message.substr(start, end - start).c_str();
        // ESP_LOGD(TAG, tmp_Raumstat.c_str());
        temp_Raumstat->publish_state(GetFloatTemp(tmp_Raumstat));

        // Nach den Temperaturen die Eingänge abfragen
        send_cmd_("1200");
    }

    if ((message.find("1200")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "1200 found -> Inputs");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // 1200;6;1;1;0;1;1;0
        std::string tmp_in;
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Abtau_Soledruck_Durchfluss  = new Sensor();  // 1200/2
        tmp_in = message.substr(start, end - start).c_str();
        ein_Abtau_Soledruck_Durchfluss->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Sperrzeit_EVU               = new Sensor();  // 1200/3
        tmp_in = message.substr(start, end - start).c_str();
        ein_Sperrzeit_EVU->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Hochdruckpressostat         = new Sensor();  // 1200/4
        tmp_in = message.substr(start, end - start).c_str();
        ein_Hochdruckpressostat->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Motorschutz                 = new Sensor();  // 1200/5
        tmp_in = message.substr(start, end - start).c_str();
        ein_Motorschutz->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Niederdruckpressostat       = new Sensor();  // 1200/6
        tmp_in = message.substr(start, end - start).c_str();
        ein_Niederdruckpressostat->publish_state(GetInputOutputState(tmp_in));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *ein_Fremdstromanode             = new Sensor();  // 1200/7
        tmp_in = message.substr(start, end - start).c_str();
        ein_Fremdstromanode->publish_state(GetInputOutputState(tmp_in));

        // Nach den Eingängen die Ausgänge abfragen
        send_cmd_("1300");
    }

    if ((message.find("1300")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "1300 found -> Outputs");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // 1300;13;0;0;1;1;0;1;0;0;0;0;0;0;0
        size_t start = 5;
        size_t end = message.find(';', start);
        std::string tmp_out;
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ATV             = new Sensor();  // 1300/2
        tmp_out = message.substr(start, end - start).c_str();
        aus_ATV->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_BWP             = new Sensor();  // 1300/3
        tmp_out = message.substr(start, end - start).c_str();
        aus_BWP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_FBHP            = new Sensor();  // 1300/4
        tmp_out = message.substr(start, end - start).c_str();
        aus_FBHP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_HZP             = new Sensor();  // 1300/5
        tmp_out = message.substr(start, end - start).c_str();
        aus_HZP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Mischer_1_Auf   = new Sensor();  // 1300/6
        tmp_out = message.substr(start, end - start).c_str();
        aus_Mischer_1_Auf->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Mischer_1_Zu    = new Sensor();  // 1300/7
        tmp_out = message.substr(start, end - start).c_str();
        aus_Mischer_1_Zu->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_VentWP          = new Sensor();  // 1300/8
        tmp_out = message.substr(start, end - start).c_str();
        aus_VentWP->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_VentBrunnen     = new Sensor();  // 1300/9
        tmp_out = message.substr(start, end - start).c_str();
        aus_VentBrunnen->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Verdichter_1    = new Sensor();  // 1300/10
        tmp_out = message.substr(start, end - start).c_str();
        aus_Verdichter_1->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_Verdichter_2    = new Sensor();  // 1300/11
        tmp_out = message.substr(start, end - start).c_str();
        aus_Verdichter_2->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZPumpe          = new Sensor();  // 1300/12
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZPumpe->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZWE             = new Sensor();  // 1300/13
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZWE->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor *aus_ZWE_Stoerung    = new Sensor();  // 1300/14
        tmp_out = message.substr(start, end - start).c_str();
        aus_ZWE_Stoerung->publish_state(GetInputOutputState(tmp_out));

        // Nach den Ausgänen den Anlagenstatus abfragen
        send_cmd_("1700");
    }

    if ((message.find("1700")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "1700 found -> Status");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // size_t message_start = 0;
        // size_t message_end = 50;
	      // std::string message_out;
	      // message_out = message.substr(message_start,message_end).c_str();
  	    // ESP_LOGD(TAG, "Message: %s", message_out.c_str());
        // Anlagenstatus
        // 1700;12;2;V2.33;1;5;19;12;8;10;42;12;0;1
	      // 1700;12;14;V2.33;1;0;19;2;22;7;29;22,0,0
        size_t start = 5;
        size_t end = message.find(';', start);
        std::string tmp_out;
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor Anlagentyp              = new Sensor();  // 1700/2
        tmp_out = message.substr(start, end - start).c_str();
        ESP_LOGD(TAG, "Anlagentyp: %s", tmp_out.c_str());
        status_Anlagentyp->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor Softwareversion         = new Sensor();  // 1700/3
        tmp_out = message.substr(start, end - start).c_str();
        ESP_LOGD(TAG, "Softwareversion: %s", tmp_out.c_str());
	      // status_Softwareversion->publish_state(GetInputOutputState(tmp_out));
	      // status_Softwareversion->publish_state(tmp_out);
        start = end + 1;
        end = message.find(';', start);
        // Sensor Bivalenzstufe           = new Sensor();  // 1700/4
        tmp_out = message.substr(start, end - start).c_str();
        ESP_LOGD(TAG, "Bivalenzstufe: %s", tmp_out.c_str());
	      status_Bivalenzstufe->publish_state(GetInputOutputState(tmp_out));
        start = end + 1;
        end = message.find(';', start);
        // Sensor Betriebszustand         = new Sensor();  // 1700/5
        tmp_out = message.substr(start, end - start).c_str();
        ESP_LOGD(TAG, "Betriebszustand: %s", tmp_out.c_str());
	      status_Betriebszustand->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        //  Sensor *Startdatum_Tag      = new Sensor();  // 1700/6
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startdatum_Tag->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Startdatum_Monat    = new Sensor();  // 1700/7
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startdatum_Monat->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Startdatum_Jahr     = new Sensor();  // 1700/8
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startdatum_Jahr->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Startuhrzeit_Std    = new Sensor();  // 1700/9
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startuhrzeit_Std->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Startuhrzeit_Min    = new Sensor();  // 1700/10
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startuhrzeit_Min->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Startuhrzeit_Sek    = new Sensor();  // 1700/11
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Startuhrzeit_Sek->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Compact             = new Sensor();  // 1700/12
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Compact->publish_state(GetInputOutputState(tmp_out));
        // start = end + 1;
        // end = message.find(';', start);
        //  Sensor *Comfort             = new Sensor();  // 1700/13
        // tmp_out = message.substr(start, end - start).c_str();
        // status_Comfort->publish_state(GetInputOutputState(tmp_out));

        // Nach den Stati den Heizungsmodus abfragen
        // Heizungsmodus
        send_cmd_("3405");
    }

    if ((message.find("3405")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "3405 found -> Heizungsmodus");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // 3405;1;4
        std::string tmp_in;
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        // Sensor *modus_Heizung     = new Sensor();  // 3405/2
        tmp_in = message.substr(start, end - start).c_str();
        modus_Heizung->publish_state(GetInputOutputState(tmp_in));

        // Nach dem Heizungsmodus den Warmwassermodus abfragen
        send_cmd_("3505");
    }

    if ((message.find("3505")==0)&&(message.length()>4)){
        ESP_LOGD(TAG, "3505 found -> Warmwassermodus");
        ESP_LOGD(TAG, "R: %s - %d", message.c_str(), 0);
        // 3505;1;0
        std::string tmp_in;
        size_t start = 5;
        size_t end = message.find(';', start);
        // Anzahl Elemente -> wird nicht verwendet
        start = end + 1;
        end = message.find(';', start);
        //  Sensor *modus_Warmwasser  = new Sensor();  // 3505/2
        tmp_in = message.substr(start, end - start).c_str();
        modus_Warmwasser->publish_state(GetInputOutputState(tmp_in));
    }
    
    if (message.empty())
        return;
  }

  CallbackManager<void(std::string, std::string)> callback_;
};

} // namespace esphome

