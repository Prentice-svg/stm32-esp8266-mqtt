<template>
	<view class="tab-shell">
		<view class="tab-bar">
			<view
				v-for="item in tabs"
				:key="item.key"
				class="tab-item"
				:class="{ active: active === item.key }"
				@tap="go(item)"
			>
				<text class="tab-icon">{{ item.icon }}</text>
				<text class="tab-text">{{ item.text }}</text>
			</view>
		</view>
	</view>
</template>

<script>
export default {
	name: 'AppTabBar',
	props: {
		active: {
			type: String,
			required: true
		}
	},
	data() {
		return {
			tabs: [
				{ key: 'monitor', text: '监测', icon: '⌁', url: '/pages/monitor/monitor' },
				{ key: 'history', text: '历史', icon: '□', url: '/pages/history/history' },
				{ key: 'settings', text: '设置', icon: '⚙', url: '/pages/settings/settings' }
			]
		}
	},
	methods: {
		go(item) {
			if (item.key === this.active) return
			uni.reLaunch({
				url: item.url
			})
		}
	}
}
</script>

<style>
.tab-shell {
	position: fixed;
	left: 0;
	right: 0;
	bottom: 0;
	z-index: 20;
	padding: 14rpx 32rpx calc(18rpx + env(safe-area-inset-bottom));
	background: rgba(246, 249, 252, 0.82);
	backdrop-filter: blur(18px);
}

.tab-bar {
	height: 104rpx;
	padding: 10rpx;
	border-radius: 36rpx;
	background: #FFFFFF;
	box-shadow: 0 -8rpx 34rpx rgba(15, 23, 42, 0.1);
	display: flex;
	border: 1rpx solid rgba(226, 232, 240, 0.9);
}

.tab-item {
	flex: 1;
	border-radius: 28rpx;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	gap: 4rpx;
	color: #94A3B8;
	font-weight: 800;
}

.tab-item.active {
	color: #2563EB;
	background: rgba(37, 99, 235, 0.1);
}

.tab-icon {
	font-size: 28rpx;
	line-height: 1;
}

.tab-text {
	font-size: 22rpx;
	line-height: 1.2;
}
</style>
