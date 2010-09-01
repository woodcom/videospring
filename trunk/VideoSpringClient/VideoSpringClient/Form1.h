#pragma once

namespace VideoSpringClient {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^  Server;
	protected: 

	protected: 
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::TextBox^  Port;

	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::ComboBox^  AudioDevice;

	private: System::Windows::Forms::ComboBox^  VideoDevice;

	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Button^  ConnectButton;
	private: System::Windows::Forms::TextBox^  TextLog;



	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->Server = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->Port = (gcnew System::Windows::Forms::TextBox());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->ConnectButton = (gcnew System::Windows::Forms::Button());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->VideoDevice = (gcnew System::Windows::Forms::ComboBox());
			this->AudioDevice = (gcnew System::Windows::Forms::ComboBox());
			this->TextLog = (gcnew System::Windows::Forms::TextBox());
			this->groupBox1->SuspendLayout();
			this->SuspendLayout();
			// 
			// Server
			// 
			this->Server->Location = System::Drawing::Point(59, 12);
			this->Server->Name = L"Server";
			this->Server->Size = System::Drawing::Size(221, 20);
			this->Server->TabIndex = 0;
			this->Server->Text = L"127.0.0.1";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(12, 15);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(41, 13);
			this->label1->TabIndex = 1;
			this->label1->Text = L"Server:";
			this->label1->Click += gcnew System::EventHandler(this, &Form1::label1_Click);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(24, 42);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(29, 13);
			this->label2->TabIndex = 2;
			this->label2->Text = L"Port:";
			// 
			// Port
			// 
			this->Port->Location = System::Drawing::Point(59, 39);
			this->Port->Name = L"Port";
			this->Port->Size = System::Drawing::Size(47, 20);
			this->Port->TabIndex = 3;
			this->Port->Text = L"1234";
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->AudioDevice);
			this->groupBox1->Controls->Add(this->VideoDevice);
			this->groupBox1->Controls->Add(this->label4);
			this->groupBox1->Controls->Add(this->label3);
			this->groupBox1->Location = System::Drawing::Point(13, 65);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(267, 87);
			this->groupBox1->TabIndex = 4;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Devices";
			// 
			// ConnectButton
			// 
			this->ConnectButton->Location = System::Drawing::Point(12, 158);
			this->ConnectButton->Name = L"ConnectButton";
			this->ConnectButton->Size = System::Drawing::Size(268, 23);
			this->ConnectButton->TabIndex = 5;
			this->ConnectButton->Text = L"Connect";
			this->ConnectButton->UseVisualStyleBackColor = true;
			this->ConnectButton->Click += gcnew System::EventHandler(this, &Form1::ConnectButton_Click);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(3, 23);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(37, 13);
			this->label3->TabIndex = 0;
			this->label3->Text = L"Video:";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(3, 51);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(37, 13);
			this->label4->TabIndex = 1;
			this->label4->Text = L"Audio:";
			// 
			// VideoDevice
			// 
			this->VideoDevice->FormattingEnabled = true;
			this->VideoDevice->Location = System::Drawing::Point(46, 20);
			this->VideoDevice->Name = L"VideoDevice";
			this->VideoDevice->Size = System::Drawing::Size(215, 21);
			this->VideoDevice->TabIndex = 2;
			// 
			// AudioDevice
			// 
			this->AudioDevice->FormattingEnabled = true;
			this->AudioDevice->Location = System::Drawing::Point(46, 48);
			this->AudioDevice->Name = L"AudioDevice";
			this->AudioDevice->Size = System::Drawing::Size(215, 21);
			this->AudioDevice->TabIndex = 3;
			// 
			// TextLog
			// 
			this->TextLog->AcceptsReturn = true;
			this->TextLog->AcceptsTab = true;
			this->TextLog->Location = System::Drawing::Point(12, 187);
			this->TextLog->Multiline = true;
			this->TextLog->Name = L"TextLog";
			this->TextLog->ReadOnly = true;
			this->TextLog->Size = System::Drawing::Size(268, 202);
			this->TextLog->TabIndex = 6;
			this->TextLog->WordWrap = false;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(292, 401);
			this->Controls->Add(this->TextLog);
			this->Controls->Add(this->ConnectButton);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->Port);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->Server);
			this->MaximizeBox = false;
			this->MaximumSize = System::Drawing::Size(300, 428);
			this->MinimumSize = System::Drawing::Size(300, 428);
			this->Name = L"Form1";
			this->Text = L"VideoSpring";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
			 }
private: System::Void ConnectButton_Click(System::Object^  sender, System::EventArgs^  e) {
			 sender.;
		 }
};
}

