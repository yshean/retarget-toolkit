using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace Eye_Tracker_Component_C_Sharp_NET
{
	/// <summary>
	/// Summary description for GazeForm.
	/// </summary>
	public class GazeForm : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public GazeForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.SuspendLayout();
            // 
            // GazeForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(40, 40);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "GazeForm";
            this.Load += new System.EventHandler(this.GazeForm_Load);
            this.ResumeLayout(false);

		}
		#endregion

        private void GazeForm_Load(object sender, EventArgs e)
        {

        }
	}
}
