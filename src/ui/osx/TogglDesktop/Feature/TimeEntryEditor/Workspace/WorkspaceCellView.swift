//
//  WorkspaceCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class WorkspaceCellView: NSTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var contentContainerView: NSBox!
    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
        initTracking()
    }

    func render(_ item: Workspace) {
        titleTextField.stringValue = item.name
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        NSCursor.arrow.set()
        contentContainerView.animator().alphaValue = 0.0
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        NSCursor.pointingHand.set()
        contentContainerView.animator().alphaValue = 1.0
    }

    fileprivate func initCommon() {
        contentContainerView.alphaValue = 0
    }

    fileprivate func initTracking() {
        let trackingArea = NSTrackingArea(rect: bounds,
                                          options: [.activeAlways, .inVisibleRect, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        addTrackingArea(trackingArea)
    }
}
