using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;
using ImageProcessing;

namespace ImageSequenceLoaderDialog
{
    public partial class Form1 : Form
    {

        public Form1()
        {
            InitializeComponent();
                            
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            ObtCImagePro = new CImageProcessing();

            // Initialize Structure
            ObjShotInfo = new ShotInfoManaged();
            ObjShotInfo.shotList = new ArrayList();
            ObjShotInfo.shotCount = 0;
                
            Load_ShotViewTree();

            bObjCreatedExist = false;

        }

        // Load Shot view Tree
        private void Load_ShotViewTree() 
        {

            //ShotTreeView.CheckBoxes = true;
            
            //
            // This is the first node in the view.
            //
            TreeNode treeNode = new TreeNode("Frame1");
            CompleteTreeView.Nodes.Add(treeNode);
            //
            // Another node following the first node.
            //
            treeNode = new TreeNode("Frame2");
            CompleteTreeView.Nodes.Add(treeNode);
            //
            // Create two child nodes and put them in an array.
            // ... Add the third node, and specify these as its children.
            //
            TreeNode node2 = new TreeNode("Frame3");
            TreeNode node3 = new TreeNode("Frame4");
            TreeNode[] array = new TreeNode[] { node2, node3 };
            //
            // Final node.
            //
            treeNode = new TreeNode("ShotA", array);
            CompleteTreeView.Nodes.Add(treeNode);

            
        }


        //  Load frames into Complete Tree View , original tree view
        private void Load_FrametoViewTree(string[] filename)
        {
            CompleteTreeView.Nodes.Clear();
            
            foreach (string fileName in filename)
            {
                string[] filenameArray = fileName.Split('\\');
                long flong = filenameArray.LongLength;

                CompleteTreeView.Nodes.Add(new TreeNode(Convert.ToString(filenameArray.GetValue(flong-1))));
            }
        }

        //  Load frames into Shot Tree View
        private void Load_FrametoShotTreeViw(string[] filename)
        {

            ShottreeView.Nodes.Clear();
            
            foreach (string fileName in filename)
            {
                string[] filenameArray = fileName.Split('\\');
                long flong = filenameArray.LongLength;

                ShottreeView.Nodes.Add(new TreeNode(Convert.ToString(filenameArray.GetValue(flong - 1))));
            }
        }
            

        // Load Shot from config file
        private void LoadShotbtn_Click(object sender, EventArgs e)
        {
            // Init ObjShotInfo
            InitObjShotInfo();

            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                ObtCImagePro.LoadShotFromFileName(openFileDialog1.FileName);
            }

            ObjShotInfo = ObtCImagePro.GetShotInfoToCSharp();

            // Update StructureTreeView
            UpdateStructureTreeView();
        }


        // Load Video Frames
        private void LoadVideobtn_Click(object sender, EventArgs e)
        {            
            
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                this.Text = folderBrowserDialog1.SelectedPath;
                //CImageProcessing ObtCImagePro = new CImageProcessing();
                int iFilesNumber = 0;

                // Pass folder name to ImageSequenceProcessing
                ObtCImagePro.SetFolder(folderBrowserDialog1.SelectedPath);

                // Process the list of files found in the directory and pass to ImageSequenceProcessing
                string[] fileEntries = Directory.GetFiles(folderBrowserDialog1.SelectedPath);
                string allFiles = "";

                iFilesNumber = (int)fileEntries.LongLength;
                this.TotalFrameNo = iFilesNumber;

                foreach (string fileName in fileEntries)
                {
                    // retrieve file names
                    //string[] filename = fileName.Split('\\');
                    allFiles += fileName + ";";
                    Console.WriteLine(fileName);
                }

                ObtCImagePro.SetFileList(allFiles, iFilesNumber);

                // Set current frame no as 0
                this.FrameNo = 0;

                // Display file name
                string test;

                test = Convert.ToString(FrameNo);
                this.label3.Text = test;

                // display frame no
                this.label4.Text = test;

                //Display frame 0
                ObtCImagePro.DisplayFrame(FrameNo);

                Load_FrametoViewTree(fileEntries);

                Load_FrametoShotTreeViw(fileEntries);

            }

            
        }


        // Back to previous frame
        private void PreFramebtn_Click(object sender, EventArgs e)
        {
            string Message = "";

            string test;

            test = Convert.ToString(FrameNo);
            this.label3.Text = test;

            // display frame no
            this.label4.Text = test;

            if (this.FrameNo == 0)
                Message = "It's currently display beginning frame";
            else
            {
                this.FrameNo -= 1;
                //Display frame 0
                ObtCImagePro.DisplayFrame(FrameNo);
            }
        }


        // Go to next frame
        private void NextFramebtn_Click(object sender, EventArgs e)
        {
            this.FrameNo += 1;

            string test;

            test = Convert.ToString(FrameNo);
            this.label3.Text = test;

            // display frame no
            this.label4.Text = test;

            //Display frame 0
            ObtCImagePro.DisplayFrame(FrameNo);
        }


        // Save Shot configuration to config file
        private void SaveShotbtn_Click(object sender, EventArgs e)
        {
            //Stream myStream;
            saveFileDialog1.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
            saveFileDialog1.FilterIndex = 2;
            saveFileDialog1.RestoreDirectory = true;

            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {

                /*
                if ((myStream = saveFileDialog1.OpenFile()) != null)
                {
                    StreamWriter wText = new StreamWriter(myStream);

                    wText.Write(" your text");

                    myStream.Close();
                }
                */
            }

            //Test();
            
            // Pass Shot Info Object
            ObtCImagePro.PassShotInfoToCPlusManaged(ObjShotInfo);
     
            ObtCImagePro.SaveShotToFileName(saveFileDialog1.FileName);
      
        }


        private void JumpFramebtn_Click(object sender, EventArgs e)
        {
            string sFrameNo = textBox1.Text;
            this.FrameNo = Convert.ToInt32(sFrameNo);

            string test;

            test = Convert.ToString(FrameNo);
            this.label3.Text = test;

            // display frame no
            this.label4.Text = test;

            //Display frame X
            ObtCImagePro.DisplayFrame(FrameNo);
        }

        private void Playbtn_Click(object sender, EventArgs e)
        {
            int i = 0;
            
            //Display frame 0
            for(i=0; i<= this.TotalFrameNo; i++)
                ObtCImagePro.DisplayFrame(i);

        }


        private void InitObjShotInfo()
        {
            // Initialize Structure
            ObjShotInfo = new ShotInfoManaged();
            ObjShotInfo.shotList = new ArrayList();
            ObjShotInfo.shotCount = 0;
        }


        private void ShotTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            //Display frame X
            ObtCImagePro.DisplayFrame(CompleteTreeView.SelectedNode.Index);
        }


        // Add Shot
        private void AddShot()
        {

            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();

            // Create Shotlist array
            ObjShotInfo.shotList.Add(new ShotManaged());

            ObjShotInfo.shotCount = ObjShotInfo.shotList.Count;

            // Add Dummy SubShot for initialization purpose
            // Initialize SubShotList, it's important
            TempShot.subShotList = new ArrayList();

            // Add temp subshot to temp shot object 
            TempShot.subShotList.Add(new SubShotManaged());
            
            // Store back to Shotlist
            ObjShotInfo.shotList[ObjShotInfo.shotList.Count-1] = TempShot;

        }


        // Add SubShot
        private void AddSubShot(int ShotNo, int shotType, int start, int end)
        {
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            
            // Create a temp SubShot and assign value
            SubShotManaged TempSubShot = new SubShotManaged();

            TempSubShot.shotType = shotType;
            TempSubShot.start = start;
            TempSubShot.end = end;

            // Retrive current shot parameter and copy to temp shot object
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);


            // Remove first dummy shot if subshot count = 0
            if (TempShot.subShotCount == 0)
                TempShot.subShotList.RemoveAt(TempShot.subShotList.Count-1);

            // Add temp subshot to temp shot object 
            TempShot.subShotList.Add(TempSubShot);
            
            TempShot.subShotCount = TempShot.subShotList.Count;

            ObjShotInfo.shotList[ShotNo] = TempShot;
            
        }



        // Remove Last Shot
        private void RemoveShot()
        {
            // Create Shotlist array
            ObjShotInfo.shotList.RemoveAt(ObjShotInfo.shotList.Count-1);

            ObjShotInfo.shotCount = ObjShotInfo.shotList.Count;

        }


        // Remove Shot
        private void RemoveShot(int ShotNo)
        {
            // Create Shotlist array
            ObjShotInfo.shotList.RemoveAt(ShotNo);

            ObjShotInfo.shotCount = ObjShotInfo.shotList.Count;

        }


        // Remove SubShot
        private void RemoveSubShot(int ShotNo, int SubShotNo)
        {
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
 
            // Retrive current shot parameter and copy to temp shot object
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);

            TempShot.subShotList.RemoveAt(SubShotNo);
            TempShot.subShotCount = TempShot.subShotList.Count;

            ObjShotInfo.shotList[ShotNo] = TempShot;

        }


        // Remove Last SubShot
        private void RemoveSubShot(int ShotNo)
        {
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();

            // Retrive current shot parameter and copy to temp shot object
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);

            TempShot.subShotList.RemoveAt(TempShot.subShotList.Count-1);
            TempShot.subShotCount = TempShot.subShotList.Count;

            ObjShotInfo.shotList[ShotNo] = TempShot;

        }

       

        // Test
        private void Test()
        {

            // Init  
            InitObjShotInfo();

            // Add Shot 0
            AddShot();

            // Add Shot 1
            AddShot();

            // Add Shot 2
            AddShot();


            //Add subshot (frame 0 to 10) in Shot 0
            AddSubShot(0, 0, 0, 10);

            //Add subshot (frame 11 to 20) in Shot 0
            AddSubShot(0, 0, 11, 20);

            //Add subshot (frame 21 to 40) in Shot 0
            AddSubShot(0, 0, 21, 40);

            //Add subshot (frame 41 to 50) in Shot 1
            AddSubShot(1, 0, 41, 50);

            //Add subshot (frame 51 to 60) in Shot 1
            AddSubShot(1, 0, 51, 60);

            //Add subshot (frame 61 to 70) in Shot 1
            AddSubShot(1, 0, 61, 70);

            //Add subshot (frame 41 to 50) in Shot 2
            AddSubShot(2, 0, 71, 80);

            //Add subshot (frame 51 to 60) in Shot 2
            AddSubShot(2, 0, 81, 90);

            //Add subshot (frame 61 to 70) in Shot 2
            AddSubShot(2, 0, 91, 100);

            //Remove Shot 1 SubShot 1
            //RemoveSubShot(1, 1);

            //Remove Shot 2
            //RemoveShot(0);

            UpdateStructureTreeView();
     
        }



        private void CreateShotbtn_Click(object sender, EventArgs e)
        {

            int ShotNo = 0;
            string message = "Shots created";

            // Create Shot Info
            if (0 == ObjShotInfo.shotList.Count)
            {
                CreateShotbtn.Text = "Add Shot";

                // Add Shot
                // Init  
                InitObjShotInfo();

                // Add Shot 0
                AddShot();

                // Add Shot Number to Combo Box
                ShotCbox.Items.Clear();

                ShotCbox.Items.Add(ObjShotInfo.shotList.Count - 1);
 
            }
            else
            {
                AddShot();
                message = "Shot Added!";

                ShotCbox.Items.Add(ObjShotInfo.shotList.Count - 1);
                
            }
                       
            MessageBox.Show(message, "Shot Info",
            MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

            // Update StructureTreeView
            UpdateStructureTreeView();

        }

        

        private void AddSubShotBtn_Click(object sender, EventArgs e)
        {
            
            // Add the subShot
            AddSubShot(ShotCbox.SelectedIndex, 0 , Convert.ToInt32(StartSubShotFramebtn.Text), Convert.ToInt32(EndSubShotFramebtn.Text));
           
            // Update the message
            string message = "Shot ";
            message += ShotCbox.SelectedIndex.ToString();
            message += " SubShot ";
            message += SubShotCBox.Items.Count.ToString();
            message += " from Frame ";
            message += StartSubShotFramebtn.Text;
            message += " to ";
            message += EndSubShotFramebtn.Text;
            message += " added";

            MessageBox.Show(message, "Shot Created Information",
            MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            
            // Add SubShot Number to Combo Box
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            TempShot.subShotList = new ArrayList();
            
            // Retrieve current shot information
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotCbox.SelectedIndex]);

            SubShotCBox.Items.Add(TempShot.subShotList.Count - 1);

            // Update StructureTreeView
            UpdateStructureTreeView();
           
        }


        // Change display of shot\subshot item number
        private void ShotCbox_SelectedIndexChanged(object sender, EventArgs e)
        {
            int i = 0;

            // Add SubShot Number to Combo Box
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            TempShot.subShotList = new ArrayList();
            SubShotManaged TempSubShot = new SubShotManaged();

            // Retrieve current shot information
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotCbox.SelectedIndex]);
       
            // Clean up old data
            SubShotCBox.Items.Clear();

            // Update SubShot Combo box
            for (i = 0; i < TempShot.subShotCount; i++)
                SubShotCBox.Items.Add(i);

        }


        private void creatNoteView(int ShotNo, int startFrame, int endFrame)
        {

            string sShot = "Shot";
            
            sShot += ShotNo.ToString();

            TreeNode node1 = new TreeNode("0000.bmp");
            TreeNode node2 = new TreeNode("0001.bmp");
            TreeNode[] array = new TreeNode[] {  node1, node2  };
            
            TreeNode treeNode = new TreeNode(sShot, array);
            //ShotTreeView.Nodes.Add(treeNode);
            CompleteTreeView.Nodes.Insert(startFrame, "Shot0");

            CompleteTreeView.Nodes.Insert(startFrame, new TreeNode(sShot));
                 
        }


        // Remove sub Shot Button
        private void RemoveSubShotbtn_Click(object sender, EventArgs e)
        {

            string message = "SubShot Removed";

            if (-1 == SubShotCBox.SelectedIndex)
            {
                RemoveSubShot(ShotCbox.SelectedIndex);
                SubShotCBox.Items.RemoveAt(SubShotCBox.Items.Count - 1);
 
            }else
            {
                RemoveSubShot(ShotCbox.SelectedIndex, SubShotCBox.SelectedIndex);
                SubShotCBox.Items.RemoveAt(SubShotCBox.SelectedIndex);
           }
            
            //Remove SubShot
            MessageBox.Show(message, "Shot Info",
            MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

            // Update StructureTreeView
            UpdateStructureTreeView();
        }


        // Remove Shot Button
        private void RemoveShotbtn_Click(object sender, EventArgs e)
        {
            string message = "Shot Removed";

            // Update Shot Combo Box
            ShotCbox.Items.RemoveAt(ObjShotInfo.shotList.Count-1);

            // Remove Shot
            RemoveShot();
                
            MessageBox.Show(message, "Shot Info",
            MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

            // Update StructureTreeView
            UpdateStructureTreeView();

        }


        // Play Shot/SubShot button
        private void PlayShotbtn_Click(object sender, EventArgs e)
        {
            int i = 0;
                
            //Play Shot/SubShot
            for (i = TempSelectedStartFrame; i <= TempSelectedEndFrame; i++)
                ObtCImagePro.DisplayFrame(i);

        }

        private void ShottreeView_AfterSelect_1(object sender, TreeViewEventArgs e)
        {
            //Display frame X
            ObtCImagePro.DisplayFrame(CompleteTreeView.Nodes[ShottreeView.SelectedNode.Index + TempSelectedStartFrame].Index);
   
        }


        // Update StructureTreeView 
        private void UpdateStructureTreeView()
        {

            string sShot = "Shot";
            string sSubShot = "SubShot";
            int ShotNo = 0 ;
            int SubShotNo = 0;

            //Clear Up
            StructureTreeView.Nodes.Clear();

            // Get Shot , SubShot and Iterate on each
            
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            TempShot.subShotList = new ArrayList();
          
             // Iterate Shot and uppdate to tree view
            for(ShotNo =0; ShotNo< ObjShotInfo.shotList.Count; ShotNo++)
            {
                 // Tree Node
                 TreeNode ShotTreeNode = new TreeNode(sShot += ShotNo.ToString());

                 // Get Shot Object
                 TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);
     
                 // Iterate SubShot and uppdate to tree view
                 for(SubShotNo =0; SubShotNo < TempShot.subShotList.Count; SubShotNo++)
                 {
                     // Add subShot Tree Node                  
                     ShotTreeNode.Nodes.Add(sSubShot += SubShotNo.ToString());

                     // init sSubShot
                     sSubShot = "SubShot";
                 }


                 StructureTreeView.Nodes.Add(ShotTreeNode);

                 // Init sShot
                 sShot = "Shot";
                 

            }
            
                  
        }


        // Update Type, Start & Frame
        private void UpdateStartEndFrameText(int ShotNo, int SubShotNo)
        {

            // Add SubShot Number to Combo Box
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            TempShot.subShotList = new ArrayList();

            // Create a temp SubShot and assign value
            SubShotManaged TempSubShot = new SubShotManaged();

            // Retrieve current shot information
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);

            if (SubShotNo != -99)
            {
                // Retrive current sub shot information
                TempSubShot = (SubShotManaged)TempShot.subShotList[SubShotNo];
                
                // Update Text
                StartSubShotFramebtn.Text = TempSubShot.start.ToString();
                EndSubShotFramebtn.Text = TempSubShot.end.ToString();

            }
            else 
            {
                // Retrive 0 sub shot information and its start frame
                TempSubShot = (SubShotManaged)TempShot.subShotList[0];

                // Update Text
                StartSubShotFramebtn.Text = TempSubShot.start.ToString();

                // Retrive last sub shot information
                TempSubShot = (SubShotManaged)TempShot.subShotList[TempShot.subShotList.Count-1];

                EndSubShotFramebtn.Text = TempSubShot.end.ToString();

            
            }

        
        }


        private void SubShotCBox_SelectedIndexChanged(object sender, EventArgs e)
        {
             // Call update start and end frame text
             UpdateStartEndFrameText(ShotCbox.SelectedIndex, SubShotCBox.SelectedIndex);

        }

        private void StructureTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
        
            string sShot = "";
            string sSubShot = "";

            TreeNode ParentTree = new TreeNode();
            ParentTree.Text = "";
            ParentTree = StructureTreeView.SelectedNode.Parent;


            if (ParentTree == null)
            {
                sShot = StructureTreeView.SelectedNode.Text;
                sShot = sShot.Substring(4);
                
                // Case No subshot
                sSubShot = "-99";
            }
            else
            {
                sShot = ParentTree.Text;
                sSubShot = StructureTreeView.SelectedNode.Text;

                sShot = sShot.Substring(4);
                sSubShot = sSubShot.Substring(7);
            }

                // Update Text 
                UpdateStartEndFrameText(Convert.ToInt32(sShot), Convert.ToInt32(sSubShot));

                // Update ShottreeView
                UpdateShottreeView(Convert.ToInt32(sShot), Convert.ToInt32(sSubShot));
    
          }


        private void UpdateShottreeView(int ShotNo, int SubShotNo)
        {

            int FrameNo = 0;

            // Add SubShot Number to Combo Box
            // Create a temp Shot
            ShotManaged TempShot = new ShotManaged();
            TempShot.subShotList = new ArrayList();

            // Create a temp SubShot and assign value
            SubShotManaged TempSubShot = new SubShotManaged();

            // Retrieve current shot information
            TempShot = ((ShotManaged)ObjShotInfo.shotList[ShotNo]);

            if (SubShotNo != -99)
            {
                // Retrive current sub shot information
                TempSubShot = (SubShotManaged)TempShot.subShotList[SubShotNo];

                // Update Temp start frame
                TempSelectedStartFrame = TempSubShot.start;
                TempSelectedEndFrame = TempSubShot.end;
            }
            else 
            {
                // Retrive 0 sub shot information, start frame
                TempSubShot = (SubShotManaged)TempShot.subShotList[0];

                TempSelectedStartFrame = TempSubShot.start;

                // Retrive last sub shot information
                TempSubShot = (SubShotManaged)TempShot.subShotList[TempShot.subShotList.Count - 1];

                TempSelectedEndFrame = TempSubShot.end;
            
            }


            // Clear up
            ShottreeView.Nodes.Clear();

            //Display frame 0
            for (FrameNo = TempSelectedStartFrame; FrameNo <= TempSelectedEndFrame; FrameNo++)
                ShottreeView.Nodes.Add(CompleteTreeView.Nodes[FrameNo].Text);


        }
                
        
    }


 }