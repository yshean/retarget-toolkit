using ImageProcessing;
using System.Collections;

namespace ImageSequenceLoaderDialog
{
    partial class Form1
    {

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.LoadVideobtn = new System.Windows.Forms.Button();
            this.NextFramebtn = new System.Windows.Forms.Button();
            this.PreFramebtn = new System.Windows.Forms.Button();
            this.JumpFramebtn = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.SaveShotbtn = new System.Windows.Forms.Button();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.LoadShotbtn = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.Playbtn = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.ShottreeView = new System.Windows.Forms.TreeView();
            this.StructureTreeView = new System.Windows.Forms.TreeView();
            this.PlayShotbtn = new System.Windows.Forms.Button();
            this.RemoveShotbtn = new System.Windows.Forms.Button();
            this.SubShotCBox = new System.Windows.Forms.ComboBox();
            this.RemoveSubShotbtn = new System.Windows.Forms.Button();
            this.EndSubShotFramebtn = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.StartSubShotFramebtn = new System.Windows.Forms.TextBox();
            this.AddSubShotBtn = new System.Windows.Forms.Button();
            this.ShotCbox = new System.Windows.Forms.ComboBox();
            this.CreateShotbtn = new System.Windows.Forms.Button();
            this.CompleteTreeView = new System.Windows.Forms.TreeView();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // LoadVideobtn
            // 
            this.LoadVideobtn.Location = new System.Drawing.Point(15, 69);
            this.LoadVideobtn.Name = "LoadVideobtn";
            this.LoadVideobtn.Size = new System.Drawing.Size(447, 23);
            this.LoadVideobtn.TabIndex = 0;
            this.LoadVideobtn.Text = "Load Video Frames";
            this.LoadVideobtn.UseVisualStyleBackColor = true;
            this.LoadVideobtn.Click += new System.EventHandler(this.LoadVideobtn_Click);
            // 
            // NextFramebtn
            // 
            this.NextFramebtn.Location = new System.Drawing.Point(118, 98);
            this.NextFramebtn.Name = "NextFramebtn";
            this.NextFramebtn.Size = new System.Drawing.Size(104, 23);
            this.NextFramebtn.TabIndex = 1;
            this.NextFramebtn.Text = "Next Frame";
            this.NextFramebtn.UseVisualStyleBackColor = true;
            this.NextFramebtn.Click += new System.EventHandler(this.NextFramebtn_Click);
            // 
            // PreFramebtn
            // 
            this.PreFramebtn.Location = new System.Drawing.Point(15, 98);
            this.PreFramebtn.Name = "PreFramebtn";
            this.PreFramebtn.Size = new System.Drawing.Size(93, 23);
            this.PreFramebtn.TabIndex = 2;
            this.PreFramebtn.Text = "Previous Frame";
            this.PreFramebtn.UseVisualStyleBackColor = true;
            this.PreFramebtn.Click += new System.EventHandler(this.PreFramebtn_Click);
            // 
            // JumpFramebtn
            // 
            this.JumpFramebtn.Location = new System.Drawing.Point(332, 98);
            this.JumpFramebtn.Name = "JumpFramebtn";
            this.JumpFramebtn.Size = new System.Drawing.Size(93, 23);
            this.JumpFramebtn.TabIndex = 6;
            this.JumpFramebtn.Text = "Jump To ";
            this.JumpFramebtn.UseVisualStyleBackColor = true;
            this.JumpFramebtn.Click += new System.EventHandler(this.JumpFramebtn_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(435, 98);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(44, 20);
            this.textBox1.TabIndex = 7;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 43);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(36, 13);
            this.label1.TabIndex = 8;
            this.label1.Text = "Frame";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 19);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(54, 13);
            this.label2.TabIndex = 9;
            this.label2.Text = "File Name";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(74, 43);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(0, 13);
            this.label3.TabIndex = 10;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(74, 19);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(0, 13);
            this.label4.TabIndex = 11;
            // 
            // SaveShotbtn
            // 
            this.SaveShotbtn.Location = new System.Drawing.Point(118, 17);
            this.SaveShotbtn.Name = "SaveShotbtn";
            this.SaveShotbtn.Size = new System.Drawing.Size(110, 23);
            this.SaveShotbtn.TabIndex = 12;
            this.SaveShotbtn.Text = "Save Shot";
            this.SaveShotbtn.UseVisualStyleBackColor = true;
            this.SaveShotbtn.Click += new System.EventHandler(this.SaveShotbtn_Click);
            // 
            // LoadShotbtn
            // 
            this.LoadShotbtn.Location = new System.Drawing.Point(10, 17);
            this.LoadShotbtn.Name = "LoadShotbtn";
            this.LoadShotbtn.Size = new System.Drawing.Size(93, 23);
            this.LoadShotbtn.TabIndex = 13;
            this.LoadShotbtn.Text = "Load Shot";
            this.LoadShotbtn.UseVisualStyleBackColor = true;
            this.LoadShotbtn.Click += new System.EventHandler(this.LoadShotbtn_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.Playbtn);
            this.groupBox1.Controls.Add(this.JumpFramebtn);
            this.groupBox1.Controls.Add(this.textBox1);
            this.groupBox1.Controls.Add(this.LoadVideobtn);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.NextFramebtn);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.PreFramebtn);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(489, 158);
            this.groupBox1.TabIndex = 14;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Frame Test";
            // 
            // Playbtn
            // 
            this.Playbtn.Location = new System.Drawing.Point(228, 98);
            this.Playbtn.Name = "Playbtn";
            this.Playbtn.Size = new System.Drawing.Size(98, 23);
            this.Playbtn.TabIndex = 12;
            this.Playbtn.Text = "Play";
            this.Playbtn.UseVisualStyleBackColor = true;
            this.Playbtn.Click += new System.EventHandler(this.Playbtn_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.ShottreeView);
            this.groupBox2.Controls.Add(this.StructureTreeView);
            this.groupBox2.Controls.Add(this.PlayShotbtn);
            this.groupBox2.Controls.Add(this.RemoveShotbtn);
            this.groupBox2.Controls.Add(this.SubShotCBox);
            this.groupBox2.Controls.Add(this.RemoveSubShotbtn);
            this.groupBox2.Controls.Add(this.EndSubShotFramebtn);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.StartSubShotFramebtn);
            this.groupBox2.Controls.Add(this.AddSubShotBtn);
            this.groupBox2.Controls.Add(this.ShotCbox);
            this.groupBox2.Controls.Add(this.CreateShotbtn);
            this.groupBox2.Controls.Add(this.CompleteTreeView);
            this.groupBox2.Controls.Add(this.LoadShotbtn);
            this.groupBox2.Controls.Add(this.SaveShotbtn);
            this.groupBox2.Location = new System.Drawing.Point(12, 176);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(489, 355);
            this.groupBox2.TabIndex = 15;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Shot Test";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(168, 154);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(99, 13);
            this.label10.TabIndex = 35;
            this.label10.Text = "Shot/SubShot Files";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(12, 154);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(122, 13);
            this.label9.TabIndex = 34;
            this.label9.Text = "Shot Info Structure View";
            // 
            // ShottreeView
            // 
            this.ShottreeView.Location = new System.Drawing.Point(266, 170);
            this.ShottreeView.Name = "ShottreeView";
            this.ShottreeView.Size = new System.Drawing.Size(217, 175);
            this.ShottreeView.TabIndex = 33;
            this.ShottreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.ShottreeView_AfterSelect_1);
            // 
            // StructureTreeView
            // 
            this.StructureTreeView.FullRowSelect = true;
            this.StructureTreeView.Location = new System.Drawing.Point(6, 170);
            this.StructureTreeView.Name = "StructureTreeView";
            this.StructureTreeView.Size = new System.Drawing.Size(254, 175);
            this.StructureTreeView.TabIndex = 32;
            this.StructureTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.StructureTreeView_AfterSelect);
            // 
            // PlayShotbtn
            // 
            this.PlayShotbtn.Location = new System.Drawing.Point(186, 119);
            this.PlayShotbtn.Name = "PlayShotbtn";
            this.PlayShotbtn.Size = new System.Drawing.Size(113, 23);
            this.PlayShotbtn.TabIndex = 31;
            this.PlayShotbtn.Text = "Play Shot\\SubShot";
            this.PlayShotbtn.UseVisualStyleBackColor = true;
            this.PlayShotbtn.Click += new System.EventHandler(this.PlayShotbtn_Click);
            // 
            // RemoveShotbtn
            // 
            this.RemoveShotbtn.Location = new System.Drawing.Point(118, 50);
            this.RemoveShotbtn.Name = "RemoveShotbtn";
            this.RemoveShotbtn.Size = new System.Drawing.Size(110, 23);
            this.RemoveShotbtn.TabIndex = 30;
            this.RemoveShotbtn.Text = "Remove Shot";
            this.RemoveShotbtn.UseVisualStyleBackColor = true;
            this.RemoveShotbtn.Click += new System.EventHandler(this.RemoveShotbtn_Click);
            // 
            // SubShotCBox
            // 
            this.SubShotCBox.FormattingEnabled = true;
            this.SubShotCBox.Location = new System.Drawing.Point(241, 81);
            this.SubShotCBox.Name = "SubShotCBox";
            this.SubShotCBox.Size = new System.Drawing.Size(37, 21);
            this.SubShotCBox.TabIndex = 29;
            this.SubShotCBox.SelectedIndexChanged += new System.EventHandler(this.SubShotCBox_SelectedIndexChanged);
            // 
            // RemoveSubShotbtn
            // 
            this.RemoveSubShotbtn.Location = new System.Drawing.Point(118, 79);
            this.RemoveSubShotbtn.Name = "RemoveSubShotbtn";
            this.RemoveSubShotbtn.Size = new System.Drawing.Size(110, 23);
            this.RemoveSubShotbtn.TabIndex = 28;
            this.RemoveSubShotbtn.Text = "Remove SubShot";
            this.RemoveSubShotbtn.UseVisualStyleBackColor = true;
            this.RemoveSubShotbtn.Click += new System.EventHandler(this.RemoveSubShotbtn_Click);
            // 
            // EndSubShotFramebtn
            // 
            this.EndSubShotFramebtn.Location = new System.Drawing.Point(380, 81);
            this.EndSubShotFramebtn.Name = "EndSubShotFramebtn";
            this.EndSubShotFramebtn.Size = new System.Drawing.Size(33, 20);
            this.EndSubShotFramebtn.TabIndex = 27;
            this.EndSubShotFramebtn.Text = "0";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(430, 84);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(36, 13);
            this.label7.TabIndex = 26;
            this.label7.Text = "Frame";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(344, 84);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(20, 13);
            this.label8.TabIndex = 25;
            this.label8.Text = "To";
            // 
            // StartSubShotFramebtn
            // 
            this.StartSubShotFramebtn.Location = new System.Drawing.Point(293, 81);
            this.StartSubShotFramebtn.Name = "StartSubShotFramebtn";
            this.StartSubShotFramebtn.Size = new System.Drawing.Size(33, 20);
            this.StartSubShotFramebtn.TabIndex = 24;
            this.StartSubShotFramebtn.Text = "0";
            // 
            // AddSubShotBtn
            // 
            this.AddSubShotBtn.Location = new System.Drawing.Point(10, 79);
            this.AddSubShotBtn.Name = "AddSubShotBtn";
            this.AddSubShotBtn.Size = new System.Drawing.Size(93, 23);
            this.AddSubShotBtn.TabIndex = 23;
            this.AddSubShotBtn.Text = "Add SubShot";
            this.AddSubShotBtn.UseVisualStyleBackColor = true;
            this.AddSubShotBtn.Click += new System.EventHandler(this.AddSubShotBtn_Click);
            // 
            // ShotCbox
            // 
            this.ShotCbox.FormattingEnabled = true;
            this.ShotCbox.Location = new System.Drawing.Point(241, 52);
            this.ShotCbox.Name = "ShotCbox";
            this.ShotCbox.Size = new System.Drawing.Size(37, 21);
            this.ShotCbox.TabIndex = 18;
            this.ShotCbox.SelectedIndexChanged += new System.EventHandler(this.ShotCbox_SelectedIndexChanged);
            // 
            // CreateShotbtn
            // 
            this.CreateShotbtn.Location = new System.Drawing.Point(10, 50);
            this.CreateShotbtn.Name = "CreateShotbtn";
            this.CreateShotbtn.Size = new System.Drawing.Size(93, 23);
            this.CreateShotbtn.TabIndex = 15;
            this.CreateShotbtn.Text = "Create Shot";
            this.CreateShotbtn.UseVisualStyleBackColor = true;
            this.CreateShotbtn.Click += new System.EventHandler(this.CreateShotbtn_Click);
            // 
            // CompleteTreeView
            // 
            this.CompleteTreeView.Location = new System.Drawing.Point(124, 179);
            this.CompleteTreeView.Name = "CompleteTreeView";
            this.CompleteTreeView.Size = new System.Drawing.Size(98, 154);
            this.CompleteTreeView.TabIndex = 14;
            this.CompleteTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.ShotTreeView_AfterSelect);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(507, 539);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.Button LoadVideobtn;
        private System.Windows.Forms.Button NextFramebtn;
        private System.Windows.Forms.Button PreFramebtn;
        private CImageProcessing ObtCImagePro;
        private ShotInfoManaged ObjShotInfo;
        private int FrameNo;
        private int TotalFrameNo;
        private static bool bObjCreatedExist;
        private int TempSelectedStartFrame;
        private int TempSelectedEndFrame;

        private System.Windows.Forms.Button JumpFramebtn;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button SaveShotbtn;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Button LoadShotbtn;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TreeView CompleteTreeView;
        private System.Windows.Forms.Button Playbtn;
        private System.Windows.Forms.Button CreateShotbtn;
        private System.Windows.Forms.ComboBox ShotCbox;
        private System.Windows.Forms.TextBox EndSubShotFramebtn;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox StartSubShotFramebtn;
        private System.Windows.Forms.Button AddSubShotBtn;
        private System.Windows.Forms.ComboBox SubShotCBox;
        private System.Windows.Forms.Button RemoveSubShotbtn;
        private System.Windows.Forms.Button RemoveShotbtn;
        private System.Windows.Forms.Button PlayShotbtn;
        private System.Windows.Forms.TreeView StructureTreeView;
        private System.Windows.Forms.TreeView ShottreeView;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label9;
    }
}

